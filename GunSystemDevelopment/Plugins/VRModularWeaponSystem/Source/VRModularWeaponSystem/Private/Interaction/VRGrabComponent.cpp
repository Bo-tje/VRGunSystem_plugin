#include "Interaction/VRGrabComponent.h"
#include "Components/BoxComponent.h"
#include "Interaction/VRInteractor.h"
#include "GameFramework/Actor.h"
#include "Interfaces/VRInteractorInterface.h"
#include "GameFramework/PlayerController.h"
#include "Core/VRNativeTags.h"
#include "Data/VRWeaponData.h"

UVRGrabComponent::UVRGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bIsHeld = false;
	bWasSimulating = false;

	InitBoxExtent(FVector(5.0f));
	MaxGrabDistance = 12.0f;
	
	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	UPrimitiveComponent::SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	UPrimitiveComponent::SetCollisionResponseToAllChannels(ECR_Ignore);

	UPrimitiveComponent::SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UVRGrabComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UVRGrabComponent::TryGrab(UVRInteractor* Interactor)
{
	if (!Interactor) return;
	
	if (bIsHeld && CurrentInteractor.IsValid())
	{
		CurrentInteractor->RequestRelease(); 
	}

	AActor* MyOwner = GetOwner();
	
	if (bAttachOwnerOnGrab)
	{
		if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent()))
		{
			if (!bIsHeld)
			{
				bWasSimulating = RootPrim->IsSimulatingPhysics();
			}
			RootPrim->SetSimulatePhysics(false);
		}
	}
	
	bIsHeld = true;
	CurrentInteractor = Interactor;
    
	if (bAttachOwnerOnGrab)
	{
		Attach(MyOwner, Interactor);
	}
    
	SetComponentTickEnabled(true);
	LastPosition = MyOwner->GetActorLocation();
	VelocityBuffer.Empty();
    
	if (OnGrabbed.IsBound())
	{
		OnGrabbed.Broadcast(Interactor->GetOwner());
	}
    
	PlayHaptics();
}

void UVRGrabComponent::TryRelease()
{
	if (!bIsHeld) return;

	AActor* MyOwner = GetOwner();
	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent());

	if (bAttachOwnerOnGrab)
	{
		const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
		MyOwner->DetachFromActor(DetachmentRules);
    
		Throw(RootPrim);
	}
    
	bIsHeld = false;
	CurrentInteractor.Reset();
	OnGrabReleased.Broadcast();
	
	SetComponentTickEnabled(false);
}

EControllerHand UVRGrabComponent::GetHoldingHand() const
{
	if (CurrentInteractor.IsValid())
	{
		return CurrentInteractor->HandSide;
	}
	return EControllerHand::AnyHand;
}

void UVRGrabComponent::OnHoverStart_Implementation(UObject* Interactor)
{
	OnHoverStart.Broadcast(Interactor);
}

void UVRGrabComponent::OnHoverEnd_Implementation(UObject* Interactor)
{
	OnHoverEnd.Broadcast(Interactor);
}

void UVRGrabComponent::StartAction_Implementation(UObject* Interactor, float ActionValue, FGameplayTag ActionTag)
{
	if (AActor* MyOwner = GetOwner())
	{
		if (MyOwner->Implements<UVRInteractableInterface>())
		{
			IVRInteractableInterface::Execute_StartAction(MyOwner, Interactor, ActionValue, ActionTag);
		}
	}

	StartAction.Broadcast(Interactor, ActionValue, ActionTag);

	if (ActionTag.MatchesTagExact(VRNativeTags::Trigger))
	{
		OnTriggerStart.Broadcast(Interactor, ActionValue);
	}
	else if (ActionTag.MatchesTagExact(VRNativeTags::PrimaryInput))
	{
		OnPrimaryActionStart.Broadcast(Interactor, ActionValue);
	}
	else if (ActionTag.MatchesTagExact(VRNativeTags::SecondaryInput))
	{
		OnSecondaryActionStart.Broadcast(Interactor, ActionValue);
	}
}

void UVRGrabComponent::StopAction_Implementation(UObject* Interactor, FGameplayTag ActionTag)
{
	if (AActor* MyOwner = GetOwner())
	{
		if (MyOwner->Implements<UVRInteractableInterface>())
		{
			IVRInteractableInterface::Execute_StopAction(MyOwner, Interactor, ActionTag);
		}
	}

	StopAction.Broadcast(Interactor, ActionTag);

	if (ActionTag.MatchesTagExact(VRNativeTags::TriggerReleased))
	{
		OnTriggerStop.Broadcast(Interactor);
	}
	else if (ActionTag.MatchesTagExact(VRNativeTags::PrimaryInputReleased))
	{
		OnPrimaryActionStop.Broadcast(Interactor);
	}
	else if (ActionTag.MatchesTagExact(VRNativeTags::SecondaryInputReleased))
	{
		OnSecondaryActionStop.Broadcast(Interactor);
	}
}

void UVRGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CalculateVelocity(DeltaTime);

	// Break logic for sub-components (slides, handles)
	if (bIsHeld && !bAttachOwnerOnGrab && CurrentInteractor.IsValid())
	{
		float Distance = FVector::Dist(GetComponentLocation(), CurrentInteractor->GetComponentLocation());
		if (Distance > BreakDistance)
		{
			TryRelease();
		}
	}
}

void UVRGrabComponent::Attach(AActor* MyOwner, UVRInteractor* TargetInteractor) const
{
	FTransform RootTransform = MyOwner->GetRootComponent()->GetComponentTransform();
	FTransform GrabTransform = GetComponentTransform();
	FTransform GrabRelativeToRoot = GrabTransform.GetRelativeTransform(RootTransform);

	FVector GrabRelativeLocation = GrabRelativeToRoot.GetLocation();
	FRotator GrabRelativeRotation = GrabRelativeToRoot.Rotator();

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	
	if (bSnapToInteractor)
	{
		AttachmentRules.LocationRule = EAttachmentRule::SnapToTarget;
		AttachmentRules.RotationRule = EAttachmentRule::SnapToTarget;
	}
	
	MyOwner->AttachToComponent(TargetInteractor, AttachmentRules);

	if (bSnapToInteractor)
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



void UVRGrabComponent::PlayHaptics() const
{
	if (CurrentInteractor.IsValid())
	{
		CurrentInteractor->PlayHapticFeedback(GrabHapticEffect, HapticScale, bLoopHaptics);
	}
}

void UVRGrabComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	// Standard initialization logic here if needed
}

void UVRGrabComponent::InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings)
{
	if (UVRGrabSettings* GrabSettings = Cast<UVRGrabSettings>(InSettings))
	{
		GrabHapticEffect = GrabSettings->GrabHapticEffect;
		HapticScale = GrabSettings->HapticScale;
		ThrowMultiplier = GrabSettings->ThrowMultiplier;
		bSnapToInteractor = GrabSettings->bUseSocketSnap;
		MaxGrabDistance = GrabSettings->MaxGrabDistance;
		BreakDistance = GrabSettings->BreakDistance;
		GrabPoseTag = GrabSettings->AnimationGrabPoseTag;
		HoverPoseTag = GrabSettings->AnimationHoverPoseTag;
		GrabPriority = GrabSettings->GrabPriority;
		bIsMainGrip = GrabSettings->bIsMainGrip;
		bAttachOwnerOnGrab = GrabSettings->bAttachOwnerOnGrab;
		
		SetBoxExtent(GrabSettings->BoxExtents);
	}
}
