#include "Interaction/VRGrabComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/VRInteractor.h"
#include "GameFramework/Actor.h"
#include "Interfaces/VRInteractorInterface.h"
#include "GameFramework/PlayerController.h"
#include "Core/VRNativeTags.h"
#include "Data/VRWeaponData.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

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
	if (!MyOwner) return;
	
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
		if (bUseSmoothGrab && bSnapToInteractor)
		{
			bIsLerping = true;
			LerpAlpha = 0.0f;
			LerpStartTransform = MyOwner->GetActorTransform().GetRelativeTransform(Interactor->GetComponentTransform());
			
			// Attach with KeepWorld first so it follows the hand as we lerp
			FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
			MyOwner->AttachToComponent(Interactor, AttachRules);
		}
		else
		{
			Attach(MyOwner, Interactor);
		}
	}
    
	SetComponentTickEnabled(true);
	LastPosition = MyOwner->GetActorLocation();
	VelocityBuffer.Empty();
    
	if (OnGrabbed.IsBound())
	{
		OnGrabbed.Broadcast(Interactor->GetOwner());
	}
    
	if (GrabSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, GrabSound, GetComponentLocation());
	}

	PlayHaptics();
}

void UVRGrabComponent::TryRelease()
{
	if (!bIsHeld) return;

	AActor* MyOwner = GetOwner();
	if (!MyOwner) return;

	UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(MyOwner->GetRootComponent());

	if (bAttachOwnerOnGrab)
	{
		const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
		MyOwner->DetachFromActor(DetachmentRules);
    
		Throw(RootPrim);
	}
    
	bIsHeld = false;
	bIsLerping = false;
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

	// Smooth grab lerp
	if (bIsLerping && bAttachOwnerOnGrab && CurrentInteractor.IsValid())
	{
		LerpAlpha = FMath::FInterpConstantTo(LerpAlpha, 1.0f, DeltaTime, GrabLerpSpeed);
		
		EControllerHand HandSide = CurrentInteractor->HandSide;
		FTransform GripWorld = GetGripAnchorTransform(HandSide);
		FTransform ActorTransform = GetOwner()->GetActorTransform();
		FTransform GripRelative = GripWorld.GetRelativeTransform(ActorTransform);
		FTransform RelativeTarget = GripRelative.Inverse();
		
		FTransform HandTransform = CurrentInteractor->GetComponentTransform();
		
		FTransform BlendedRelativeTransform;
		BlendedRelativeTransform.Blend(LerpStartTransform, RelativeTarget, LerpAlpha);
		
		FTransform DesiredWorldTransform = BlendedRelativeTransform * HandTransform;
		GetOwner()->SetActorTransform(DesiredWorldTransform);
		
		if (LerpAlpha >= 0.99f)
		{
			bIsLerping = false;
			// Final precise snap
			Attach(GetOwner(), CurrentInteractor.Get());
		}
	}

	// Break logic for sub-components (slides, handles)
	if (bIsHeld && !bAttachOwnerOnGrab && CurrentInteractor.IsValid())
	{
		float Distance = FVector::Dist(GetComponentLocation(), CurrentInteractor->GetComponentLocation());
		if (Distance > BreakDistance)
		{
			TryRelease();
		}
	}

	// Debug Gizmos
	if (bShowDebugGizmos)
	{
		if (UWorld* World = GetWorld())
		{
			FTransform RightHandGrip = GetGripAnchorTransform(EControllerHand::Right);
			DrawDebugCoordinateSystem(World, RightHandGrip.GetLocation(), RightHandGrip.GetRotation().Rotator(), 15.0f, false, 0.0f, 0, 1.5f);

			FTransform LeftHandGrip = GetGripAnchorTransform(EControllerHand::Left);
			DrawDebugCoordinateSystem(World, LeftHandGrip.GetLocation(), LeftHandGrip.GetRotation().Rotator(), 15.0f, false, 0.0f, 0, 1.5f);
		}
	}
}

FTransform UVRGrabComponent::GetGripAnchorTransform(EControllerHand HandSide) const
{
	FName TargetSocketName = NAME_None;

	if (HandSide == EControllerHand::Right)
	{
		TargetSocketName = RightHandGripSocketName;
	}
	else if (HandSide == EControllerHand::Left)
	{
		TargetSocketName = LeftHandGripSocketName;
	}

	if (TargetSocketName.IsNone())
	{
		TargetSocketName = GripSocketName;
	}

	FTransform OutTransform;

	if (!TargetSocketName.IsNone())
	{
		// Walk up the attachment hierarchy to find a StaticMeshComponent with this socket
		USceneComponent* Parent = GetAttachParent();
		bool bFoundSocket = false;
		while (Parent)
		{
			if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Parent))
			{
				if (SMC->DoesSocketExist(TargetSocketName))
				{
					OutTransform = SMC->GetSocketTransform(TargetSocketName);
					bFoundSocket = true;
					break;
				}
			}
			Parent = Parent->GetAttachParent();
		}
		if (!bFoundSocket)
		{
			OutTransform = GetComponentTransform();
		}
	}
	else
	{
		// Fallback: use own transform (current behavior)
		OutTransform = GetComponentTransform();
	}

	// Apply rotation offset
	FRotator FinalOffset = GripRotationOffset;
	if (HandSide == EControllerHand::Right && !RightHandRotationOffset.IsNearlyZero())
	{
		FinalOffset = RightHandRotationOffset;
	}
	else if (HandSide == EControllerHand::Left && !LeftHandRotationOffset.IsNearlyZero())
	{
		FinalOffset = LeftHandRotationOffset;
	}

	if (!FinalOffset.IsNearlyZero())
	{
		FQuat OffsetQuat = FinalOffset.Quaternion();
		OutTransform.SetRotation(OutTransform.GetRotation() * OffsetQuat);
	}

	return OutTransform;
}

void UVRGrabComponent::Attach(AActor* MyOwner, UVRInteractor* TargetInteractor) const
{
	if (!MyOwner || !TargetInteractor) return;

	EControllerHand HandSide = TargetInteractor->HandSide;

	// 1. Compute the offsetted grip anchor in world space
	FTransform GripWorldTransform = GetGripAnchorTransform(HandSide);
	
	// 2. Compute the grip anchor relative to the actor root
	FTransform ActorTransform = MyOwner->GetActorTransform();
	FTransform GripRelativeToActor = GripWorldTransform.GetRelativeTransform(ActorTransform);
	
	// 3. The desired actor transform puts the grip anchor at the hand location
	FTransform HandTransform = TargetInteractor->GetComponentTransform();
	FTransform DesiredActorTransform = GripRelativeToActor.Inverse() * HandTransform;
	
	// 4. Snap if requested, then attach using KeepWorld
	if (bSnapToInteractor)
	{
		MyOwner->SetActorTransform(DesiredActorTransform);
	}
	
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepWorld, true);
	MyOwner->AttachToComponent(TargetInteractor, AttachRules);
}

void UVRGrabComponent::CalculateVelocity(float DeltaTime)
{
	if (bIsHeld && DeltaTime > 0.0f)
	{
		AActor* MyOwner = GetOwner();
		if (!MyOwner) return;

		FVector NewPosition = MyOwner->GetActorLocation();
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
		GrabSound = GrabSettings->GrabSound;
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
		
		GripSocketName = GrabSettings->GripSocketName;
		RightHandGripSocketName = GrabSettings->RightHandGripSocketName;
		LeftHandGripSocketName = GrabSettings->LeftHandGripSocketName;
		GripRotationOffset = GrabSettings->GripRotationOffset;
		RightHandRotationOffset = GrabSettings->RightHandRotationOffset;
		LeftHandRotationOffset = GrabSettings->LeftHandRotationOffset;
		bUseSmoothGrab = GrabSettings->bUseSmoothGrab;
		GrabLerpSpeed = GrabSettings->GrabLerpSpeed;

		SetBoxExtent(GrabSettings->BoxExtents);
	}
}
