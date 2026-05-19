#include "Components/VRMechanicalComponent.h"
#include "Core/VRNativeTags.h"
#include "Kismet/GameplayStatics.h"

#include "Data/VRWeaponData.h"
#include "Interaction/VRGrabComponent.h"
#include "Interaction/VRInteractor.h"
#include "Components/VRChamberComponent.h"
#include "Components/VRWeaponStateTreeComponent.h"


UVRMechanicalComponent::UVRMechanicalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MechanicalMovementType = VRNativeTags::Linear;
	LocalAxis = FVector::ForwardVector;
	MaxRange = 10.0f;
	RestingValue = 0.0f;
	HapticTickThreshold = 0.05f;
}

void UVRMechanicalComponent::BeginPlay()
{
	Super::BeginPlay();
	HomeTransform = GetRelativeTransform();

	if (USceneComponent* Parent = GetAttachParent())
	{
		ParentMotion.LastLocation = Parent->GetComponentLocation();
		ParentMotion.LastRotation = Parent->GetComponentQuat();
	}
}

void UVRMechanicalComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Always track parent motion if inertia is enabled, so it's ready when released
	if (bUseSimulatedInertia)
	{
		TrackParentMotion(DeltaTime);
	}
	
	// Safety: Check if we are actually being held by a valid interactor
	const bool bActuallyHeld = bIsBeingHeld && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor();
	
	if (bActuallyHeld)
	{
		UpdateFromHandLocation(DrivingGrabComponent->GetCurrentInteractor()->GetComponentLocation());
	}
	else
	{
		// If we were supposed to be held but the interactor is gone, clear the flag
		if (bIsBeingHeld)
		{
			bIsBeingHeld = false;
		}

		if (bUseSimulatedInertia)
		{
			CalculateInertia(DeltaTime);
		}
		
		if (bHasReturnSpring && !bIsLocked && !FMath::IsNearlyEqual(CurrentNormalisedValue, RestingValue, 0.00001f))
		{
			float SprungValue = FMath::FInterpTo(CurrentNormalisedValue, RestingValue, DeltaTime, ReturnSpeed);
			
			// Snap to resting value if we are very close to ensure events trigger
			if (FMath::IsNearlyEqual(SprungValue, RestingValue, 0.001f))
			{
				SprungValue = RestingValue;
			}
			
			SetNormalizedValue(SprungValue);
		}
	}

	if (DeltaTime > 0.0f)
	{
		CurrentNormalizedVelocity = FMath::Abs(CurrentNormalisedValue - LastFrameNormalisedValue) / DeltaTime;
		LastFrameNormalisedValue = CurrentNormalisedValue;
	}
}

void UVRMechanicalComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	// Automatically find child grab component if not set
	if (!DrivingGrabComponent)
	{
		TArray<USceneComponent*> Children;
		GetChildrenComponents(false, Children); // Immediate children only
		for (USceneComponent* Child : Children)
		{
			if (UVRGrabComponent* GrabComp = Cast<UVRGrabComponent>(Child))
			{
				DrivingGrabComponent = GrabComp;
				break;
			}
		}
	}

	if (DrivingGrabComponent)
	{
		DrivingGrabComponent->OnGrabbed.AddDynamic(this, &UVRMechanicalComponent::OnGrabbed);
		DrivingGrabComponent->OnGrabReleased.AddDynamic(this, &UVRMechanicalComponent::OnReleased);
	}
}

void UVRMechanicalComponent::InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings)
{
	if (UVRMechanicalSettings* MechSettings = Cast<UVRMechanicalSettings>(InSettings))
	{
		ApplyMechanicalSettings(MechSettings);
	}
}

void UVRMechanicalComponent::SetNormalizedValue(float NewValue)
{
	const float OldValue = CurrentNormalisedValue;
	CurrentNormalisedValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	
	// Always allow boundary hits (0 or 1) to pass through so events trigger reliably
	const bool bIsAtBoundary = FMath::IsNearlyZero(CurrentNormalisedValue) || FMath::IsNearlyEqual(CurrentNormalisedValue, 1.0f);
	if (FMath::IsNearlyEqual(OldValue, CurrentNormalisedValue, 0.00001f) && !bIsAtBoundary) return;

	OnValueChanged.Broadcast(CurrentNormalisedValue);
	
	HandleHaptics();
	CheckThresholdEvents();
	UpdateVisuals();
}

void UVRMechanicalComponent::UpdateVisuals()
{
	const float CurrentTargetOffset = CurrentNormalisedValue * MaxRange * (bInvertDirection ? -1.0f : 1.0f);
	
	if (MechanicalMovementType == VRNativeTags::Linear)
	{
		FVector NewLocation = HomeTransform.GetLocation() + LocalAxis * CurrentTargetOffset;
		SetRelativeLocation(NewLocation);
	}
	else if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		FQuat NewRotation = HomeTransform.GetRotation() * FQuat(LocalAxis, FMath::DegreesToRadians(CurrentTargetOffset));
		SetRelativeRotation(NewRotation);
	}
}

void UVRMechanicalComponent::CheckThresholdEvents()
{
	UVRWeaponStateTreeComponent* StateTree = GetOwner() ? GetOwner()->FindComponentByClass<UVRWeaponStateTreeComponent>() : nullptr;

	bool bIsRealisticSlap = false;
	if (bIsBeingHeld && CurrentNormalizedVelocity > SlapVelocityThreshold) bIsRealisticSlap = true;
	else if (!bIsBeingHeld && bHasReturnSpring && FMath::Abs(ReleaseNormalizedValue - CurrentNormalisedValue) > SlapReleaseDistanceThreshold) bIsRealisticSlap = true;
	else if (bUseSimulatedInertia && FMath::Abs(CurrentMomentum) > SlapMomentumThreshold) bIsRealisticSlap = true;

	// Reached Max
	if (CurrentNormalisedValue >= 1.0f && !bWasAtMax)
	{
		OnReachedMax.Broadcast();
		
		if (StateTree && OnReachedMaxTag.IsValid())
		{
			StateTree->SendStateTreeEvent(OnReachedMaxTag);
		}

		if (bIsRealisticSlap)
		{
			if (LimitReachedSound) UGameplayStatics::PlaySoundAtLocation(this, LimitReachedSound, GetComponentLocation());
			if (LimitReachedHapticEffect && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
			{
				DrivingGrabComponent->GetCurrentInteractor()->PlayHapticFeedback(LimitReachedHapticEffect, 0.5f);
			}
		}
		
		bWasAtMax = true;
	}
	else if (CurrentNormalisedValue < 1.0f)
	{
		bWasAtMax = false;
	}

	// Reached Min
	if (CurrentNormalisedValue <= 0.0f && !bWasAtMin)
	{
		OnReachedMin.Broadcast();

		if (StateTree && OnReachedMinTag.IsValid())
		{
			StateTree->SendStateTreeEvent(OnReachedMinTag);
		}

		if (bIsRealisticSlap)
		{
			if (LimitReachedSound) UGameplayStatics::PlaySoundAtLocation(this, LimitReachedSound, GetComponentLocation());
			if (LimitReachedHapticEffect && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
			{
				DrivingGrabComponent->GetCurrentInteractor()->PlayHapticFeedback(LimitReachedHapticEffect, 0.5f);
			}
		}

		bWasAtMin = true;
	}
	else if (CurrentNormalisedValue > 0.0f)
	{
		bWasAtMin = false;
	}
}

void UVRMechanicalComponent::HandleHaptics()
{
	if (bIsBeingHeld && MovementHapticEffect && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
	{
		if (FMath::Abs(CurrentNormalisedValue - LastHapticValue) >= HapticTickThreshold)
		{
			DrivingGrabComponent->GetCurrentInteractor()->PlayHapticFeedback(MovementHapticEffect, 0.3f);
			if (MovementSound) UGameplayStatics::PlaySoundAtLocation(this, MovementSound, GetComponentLocation());
			LastHapticValue = CurrentNormalisedValue;
		}
	}
}

void UVRMechanicalComponent::TrackParentMotion(float DeltaTime)
{
	USceneComponent* Parent = GetAttachParent();
	if (!Parent || DeltaTime <= 0.0f) return;

	const FVector CurrentLocation = Parent->GetComponentLocation();
	ParentMotion.LastVelocity = (CurrentLocation - ParentMotion.LastLocation) / DeltaTime;
	ParentMotion.LastLocation = CurrentLocation;
	
	const FQuat CurrentRot = Parent->GetComponentQuat();
	const FQuat DeltaRot = CurrentRot * ParentMotion.LastRotation.Inverse();
	ParentMotion.LastRotation = CurrentRot;

	FVector Axis; float Angle;
	DeltaRot.ToAxisAndAngle(Axis, Angle);
	if (Angle > PI) Angle -= 2.0f * PI;
	
	ParentMotion.LastAngularVelocity = Axis * (Angle / DeltaTime);
}

void UVRMechanicalComponent::CalculateInertia(float DeltaTime)
{
	if (!GetAttachParent() || DeltaTime <= 0.0f || bIsLocked) 
	{
		CurrentMomentum = 0.0f;
		return;
	}

	// Constants for inertia sensitivity
	const float LinearSensitivity = -0.0005f;
	const float RotationalSensitivity = -0.001f;
	const float MaxAccel = 20000.0f;

	float Force = 0.0f;
	const FVector WorldAxis = GetComponentTransform().TransformVectorNoScale(LocalAxis.GetSafeNormal());

	if (MechanicalMovementType == VRNativeTags::Linear)
	{
		// Derive acceleration from parent motion
		const FVector CurrentParentLoc = GetAttachParent()->GetComponentLocation();
		const FVector ParentVelocity = (CurrentParentLoc - ParentMotion.LastLocation) / DeltaTime;
		FVector Acceleration = (ParentVelocity - ParentMotion.LastVelocity) / DeltaTime;
		Acceleration = Acceleration.GetClampedToMaxSize(MaxAccel);
		
		Force = FVector::DotProduct(Acceleration, WorldAxis) * LinearSensitivity * InertiaMultiplier;
	}
	else if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		// Derive angular acceleration
		const FQuat CurrentRot = GetAttachParent()->GetComponentQuat();
		const FQuat DeltaRot = CurrentRot * ParentMotion.LastRotation.Inverse();
		FVector Axis; float Angle;
		DeltaRot.ToAxisAndAngle(Axis, Angle);
		if (Angle > PI) Angle -= 2.0f * PI;
		
		const FVector AngularVelocity = Axis * (Angle / DeltaTime);
		FVector AngularAcceleration = (AngularVelocity - ParentMotion.LastAngularVelocity) / DeltaTime;
		AngularAcceleration = AngularAcceleration.GetClampedToMaxSize(MaxAccel);
		
		Force = FVector::DotProduct(AngularAcceleration, WorldAxis) * RotationalSensitivity * InertiaMultiplier;
	}

	if (FMath::Abs(Force) > 0.1f) 
	{
		CurrentMomentum += Force * DeltaTime;
	}
	
	if (FMath::Abs(CurrentMomentum) > 0.001f)
	{
		const float ProposedValue = CurrentNormalisedValue + (CurrentMomentum * DeltaTime);
		
		if (ProposedValue >= 1.0f || ProposedValue <= 0.0f)
		{
			CurrentMomentum = 0.0f; // Hard stop at boundaries
		}
		
		SetNormalizedValue(ProposedValue);
	}
	
	CurrentMomentum = FMath::FInterpTo(CurrentMomentum, 0.0f, DeltaTime, 8.0f); // Friction
}

void UVRMechanicalComponent::SetIsLocked(bool bNewLocked)
{
	if (bIsLocked != bNewLocked)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: bIsLocked set to %s"), *GetName(), bNewLocked ? TEXT("True") : TEXT("False"))
		bIsLocked = bNewLocked;
	}
}

void UVRMechanicalComponent::SetRestingValue(float NewRestingValue)
{
	RestingValue = FMath::Clamp(NewRestingValue, 0.0f, 1.0f);
}

void UVRMechanicalComponent::AddMomentum(float MomentumAmount)
{
	CurrentMomentum += MomentumAmount;
}

void UVRMechanicalComponent::UpdateFromHandLocation(FVector HandWorldLocation)
{
	if (bIsLocked) return;

	const float CurrentRawValue = CalculateRawHandValue(HandWorldLocation);
	float DeltaRaw = 0.0f;

	if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		DeltaRaw = FMath::FindDeltaAngleDegrees(InitialGrabRawValue, CurrentRawValue);
	}
	else
	{
		DeltaRaw = CurrentRawValue - InitialGrabRawValue;
	}

	const float DirectionModifier = bInvertDirection ? -1.0f : 1.0f;
	const float NormalizedValue = GrabbedNormalizedValue + ((DeltaRaw / MaxRange) * DirectionModifier);
	
	SetNormalizedValue(NormalizedValue);
}

float UVRMechanicalComponent::CalculateRawHandValue(FVector HandWorldLocation) const
{
	if (!GetAttachParent()) return 0.0f;
	
	const FVector HandLocalSpace = GetAttachParent()->GetComponentTransform().InverseTransformPosition(HandWorldLocation);
	const FVector HandOffset = HandLocalSpace - HomeTransform.GetLocation();
	
	if (MechanicalMovementType == VRNativeTags::Linear)
	{
		return FVector::DotProduct(HandOffset, LocalAxis.GetSafeNormal());
	}
	else if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		const FVector SafeAxis = LocalAxis.GetSafeNormal();
		
		// Create a consistent reference frame around the pivot axis
		FVector RefAxis = FVector::ForwardVector;
		if (FMath::Abs(FVector::DotProduct(SafeAxis, RefAxis)) > 0.9f)
		{
			RefAxis = FVector::RightVector;
		}
		
		const FVector OrthogonalY = FVector::CrossProduct(SafeAxis, RefAxis).GetSafeNormal();
		const FVector OrthogonalX = FVector::CrossProduct(OrthogonalY, SafeAxis).GetSafeNormal();
		
		const FVector ProjectedHand = FVector::VectorPlaneProject(HandOffset, SafeAxis).GetSafeNormal();
		
		const float DotX = FVector::DotProduct(ProjectedHand, OrthogonalX);
		const float DotY = FVector::DotProduct(ProjectedHand, OrthogonalY);
		
		return FMath::RadiansToDegrees(FMath::Atan2(DotY, DotX));
	}
	
	return 0.0f;
}

void UVRMechanicalComponent::ConstructVisuals(UStaticMesh* InMesh, bool bWeldToParent, EComponentCreationMethod InCreationMethod)
{
	if (!InMesh) return;
	
	VisualMesh = NewObject<UStaticMeshComponent>(this, "VisualMesh");
	VisualMesh->CreationMethod = InCreationMethod;
	VisualMesh->SetStaticMesh(InMesh);
	
	bool bActualWeld = bWeldToParent;
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, bActualWeld);	
	
	VisualMesh->AttachToComponent(this, AttachRules);
	VisualMesh->RegisterComponent();
	
	if (bActualWeld)
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VisualMesh->SetCollisionProfileName(TEXT("PhysicsBody"));
	}
	else
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		VisualMesh->SetCollisionProfileName(TEXT("NoCollision")); 
		VisualMesh->SetGenerateOverlapEvents(false);
	}
}

void UVRMechanicalComponent::ApplyMechanicalSettings(UVRMechanicalSettings* Settings)
{
	if (!Settings) return;
    
	MechanicalMovementType = Settings->MechanicalMovementType;
	LocalAxis = Settings->LocalAxis;
	MaxRange = Settings->MaxRange;
	bInvertDirection = Settings->bInvertDirection;
	bIsLocked = Settings->bIsLocked;
	bUseSimulatedInertia = Settings->bUseSimulatedInertia;
	InertiaMultiplier = Settings->InertiaMultiplier;
	bHasReturnSpring = Settings->bHasReturnSpring;
	ReturnSpeed = Settings->ReturnSpeed;
	RestingValue = Settings->RestingValue;
	MovementHapticEffect = Settings->MovementHapticEffect;
	HapticTickThreshold = Settings->HapticTickThreshold;
	OnReachedMaxTag = Settings->OnReachedMaxTag;
	OnReachedMinTag = Settings->OnReachedMinTag;
	MovementSound = Settings->MovementSound;
	LimitReachedSound = Settings->LimitReachedSound;
	LimitReachedHapticEffect = Settings->LimitReachedHapticEffect;
	
	SlapVelocityThreshold = Settings->SlapVelocityThreshold;
	SlapReleaseDistanceThreshold = Settings->SlapReleaseDistanceThreshold;
	SlapMomentumThreshold = Settings->SlapMomentumThreshold;
}

void UVRMechanicalComponent::OnGrabbed(AActor* InteractingActor)
{
	bIsBeingHeld = true;
	
	if (DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
	{
		FVector HandWorldLocation = DrivingGrabComponent->GetCurrentInteractor()->GetComponentLocation();
		InitialGrabRawValue = CalculateRawHandValue(HandWorldLocation);
		GrabbedNormalizedValue = CurrentNormalisedValue;
	}
}

void UVRMechanicalComponent::OnReleased()
{
	bIsBeingHeld = false;
	InitialGrabRawValue = 0.0f;
	ReleaseNormalizedValue = CurrentNormalisedValue;
}
