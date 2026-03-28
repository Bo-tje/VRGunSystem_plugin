#include "VRGrabComponent.h"
#include "VRInteractor.h"
#include "GameFramework/Actor.h"
#include "VRInteractorInterface.h"
#include "GameFramework/PlayerController.h"

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
}

void UVRGrabComponent::TryGrab(UVRInteractor* Interactor)
{
	if (!Interactor) return;

	// If already held, tell the old interactor to let go so you can pass objects between hands
	if (bIsHeld && CurrentInteractor.IsValid())
	{
		CurrentInteractor->RequestRelease(); 
	}

	AActor* MyOwner = GetOwner();
	
#pragma region Set physics
	
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent()))
	{
		if (!bIsHeld)
		{
			bWasSimulating = RootPrim->IsSimulatingPhysics();
		}
		RootPrim->SetSimulatePhysics(false);
	}
	
#pragma endregion 
	
	bIsHeld = true;
	CurrentInteractor = Interactor;
    
	Attach(MyOwner, Interactor);
    
#pragma region Initialize velocity tracking
	
	SetComponentTickEnabled(true);
	LastPosition = MyOwner->GetActorLocation();
	VelocityBuffer.Empty();
    
#pragma endregion 
	
	if (OnGrabbed.IsBound())
	{
		OnGrabbed.Broadcast(Interactor->GetOwner());
	}
    
	PlayHaptics(Interactor->HandSide);
}

void UVRGrabComponent::TryRelease()
{
	if (!bIsHeld) return;

	AActor* MyOwner = GetOwner();
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent());

	const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	MyOwner->DetachFromActor(DetachmentRules);
    
	bIsHeld = false;
	Throw(RootPrim);
    
	CurrentInteractor.Reset();
	OnReleased.Broadcast();
	
	SetComponentTickEnabled(false);
}

void UVRGrabComponent::StartAction_Implementation(UObject* Interactor, EVRInteractableAction Action, float Value)
{
	// called when an action is started (e.g. trigger pressed)
}

void UVRGrabComponent::StopAction_Implementation(UObject* Interactor, EVRInteractableAction Action, float Value)
{
	// called when an action is stopped (e.g. trigger released)
}

void UVRGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CalculateVelocity(DeltaTime);
}

void UVRGrabComponent::Attach(AActor* MyOwner, UVRInteractor* TargetInteractor) const
{
	// Calculate the relative transform from the GrabComponent to the Actor Root
	FTransform GrabRelativeTransform = GetRelativeTransform();
	FVector GrabRelativeLocation = GrabRelativeTransform.GetLocation();
	FRotator GrabRelativeRotation = GrabRelativeTransform.Rotator();

	// If socket snap is not enabled, we keep the offset the player had when they grabbed
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	
	if (bUseSocketSnap)
	{
		AttachmentRules.LocationRule = EAttachmentRule::SnapToTarget;
		AttachmentRules.RotationRule = EAttachmentRule::SnapToTarget;
	}
	
	MyOwner->AttachToComponent(TargetInteractor, AttachmentRules, bUseSocketSnap ? GrabSocketName : NAME_None);

	if (bUseSocketSnap)
	{
		MyOwner->AddActorLocalOffset(-GrabRelativeLocation);
		MyOwner->AddActorLocalRotation(GrabRelativeRotation.GetInverse());
	}
}

void UVRGrabComponent::CalculateVelocity(float DeltaTime)
{
	if (bIsHeld && DeltaTime > 0.0f)
	{
		FVector NewPosition = GetOwner()->GetActorLocation();
		FVector CalculatedVelocity = (NewPosition - LastPosition) / DeltaTime;
       
		if (CalculatedVelocity.Size() < 5000.0f) 
		{
			VelocityBuffer.Add(CalculatedVelocity);
			if (VelocityBuffer.Num() > 10) VelocityBuffer.RemoveAt(0);
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

		RootPrim->SetAllPhysicsLinearVelocity(AverageVelocity * ThrowMultiplier);
	}
}

void UVRGrabComponent::PlayHaptics(const EControllerHand Side) const
{
	if (CurrentInteractor.IsValid())
	{
		if (APlayerController* PC = CurrentInteractor->GetProvidingPlayerController())
		{
			PC->PlayHapticEffect(GrabHapticEffect, Side, HapticScale, bLoopHaptics);
		}
	}
}
