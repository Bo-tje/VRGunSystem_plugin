// Fill out your copyright notice in the Description page of Project Settings.


#include "VRGrabComponent.h"

#include "VRInteractor.h"
#include "BehaviorTree/Blackboard/BlackboardKeyEnums.h"
#include "GameFramework/Actor.h"
#include "VRInteractorInterface.h"
#include "GameFramework/PlayerController.h"

// Sets default values for this component's properties
UVRGrabComponent::UVRGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bIsHeld = false;
	bWasSimulating = false;
}

void UVRGrabComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetComponentTickEnabled(false);
	
	if (AActor* MyOwner = GetOwner())
	{
		// Set the root to movable so it doesn't fail the attachment
		if (USceneComponent* Root = MyOwner->GetRootComponent())
		{
			Root->SetMobility(EComponentMobility::Movable);
		}
        
		// Ensure the object generates overlap events so the Hand can find it
		MyOwner->SetActorEnableCollision(true);
	}
}

void UVRGrabComponent::TryGrab(UVRInteractor* Interactor)
{
	if (bIsHeld || !Interactor) return;

	bIsHeld = true;
    
	// We store the Actor that owns the interactor (The Hand/Pawn)
	HoldingHand = Interactor->GetOwner(); 

	AActor* MyOwner = GetOwner();

	// Physics Handling
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent()))
	{
		bWasSimulating = RootPrim->IsSimulatingPhysics();
		RootPrim->SetSimulatePhysics(false);
	}
    
	// Use the Interactor itself as the attachment target
	Attach(MyOwner, Interactor);
    
	// Velocity tracking setup
	SetComponentTickEnabled(true);
	LastPosition = GetOwner()->GetActorLocation();
	VelocityBuffer.Empty();
    
	if (OnGrabbed.IsBound())
	{
		OnGrabbed.Broadcast(Interactor->GetOwner());
	}
    
	// Pass the side stored inside the interactor
	PlayHaptics(Interactor->HandSide);
}

void UVRGrabComponent::TryRelease()
{
	if (!bIsHeld) return;

	// References
	AActor* MyOwner = GetOwner();
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent());

	// Detach
	const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	MyOwner->DetachFromActor(DetachmentRules);
    
	bIsHeld = false;
	
	Throw(RootPrim);
    
	HoldingHand.Reset();
	OnReleased.Broadcast();
	
	SetComponentTickEnabled(false);
	
}

void UVRGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	CalculateVelocity(DeltaTime);
}

void UVRGrabComponent::Attach(AActor* MyOwner, USceneComponent* TargetComponent) const
{
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	
	if (bUseSocketSnap)
	{
		// Snapping to a specific socket
		AttachmentRules.LocationRule = EAttachmentRule::SnapToTarget;
		AttachmentRules.RotationRule = EAttachmentRule::SnapToTarget;
		AttachmentRules.ScaleRule = EAttachmentRule::KeepWorld; 
	}
	
	MyOwner->AttachToComponent(TargetComponent, AttachmentRules, bUseSocketSnap ? GrabSocketName : NAME_None);
}

void UVRGrabComponent::CalculateVelocity(float DeltaTime)
{
	// Safety check for DeltaTime
	if (bIsHeld && DeltaTime > 0.0f)
	{
		FVector NewPosition = GetOwner()->GetActorLocation();
       
		// Calculate velocity
		FVector CalculatedVelocity = (NewPosition - LastPosition) / DeltaTime;
       
		// Safety check: ignore massive spikes (teleports)
		if (CalculatedVelocity.Size() < 5000.0f) 
		{
			CurrentVelocity = CalculatedVelocity;
			VelocityBuffer.Add(CurrentVelocity);
			if (VelocityBuffer.Num() > 10) VelocityBuffer.RemoveAt(0); // Increased buffer to 10 for better smoothing
		}

		LastPosition = NewPosition;
	}
}

void UVRGrabComponent::Throw(UPrimitiveComponent* RootPrim)
{
	if (bWasSimulating && RootPrim)
	{
		RootPrim->SetSimulatePhysics(true);
		RootPrim->WakeAllRigidBodies();

		FVector AverageVelocity = FVector::ZeroVector;
		if (VelocityBuffer.Num() > 0)
		{
			for (FVector V : VelocityBuffer) AverageVelocity += V;
			AverageVelocity /= VelocityBuffer.Num();
		}

		// Overwrite any zero-velocity the engine just set during "SetSimulatePhysics"
		RootPrim->SetAllPhysicsLinearVelocity(AverageVelocity * ThrowMultiplier);
		
		UE_LOG(LogTemp, Warning, TEXT("Final Throw Velocity: %s | Buffer Count: %d"), *AverageVelocity.ToString(), VelocityBuffer.Num());
	}
}


void UVRGrabComponent::PlayHaptics(const EControllerHand Side) const
{
	if (IVRInteractorInterface* Interactor = Cast<IVRInteractorInterface>(HoldingHand->FindComponentByClass<UVRInteractor>()))
	{
		Interactor->GetProvidingPlayerController()->PlayHapticEffect(GrabHapticEffect, Side, HapticScale, bLoopHaptics);
	}
}