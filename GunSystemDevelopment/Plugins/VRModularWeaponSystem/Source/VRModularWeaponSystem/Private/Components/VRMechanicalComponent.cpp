#include "Components/VRMechanicalComponent.h"
#include "Data/VRWeaponData.h"
#include "Interaction/VRGrabComponent.h"
#include "Interaction/VRInteractor.h"

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

	// Automatically find child grab component if not set
	if (!DrivingGrabComponent)
	{
		TArray<USceneComponent*> Children;
		GetChildrenComponents(true, Children);
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
		DrivingGrabComponent->bAttachOwnerOnGrab = false; // Ensure it doesn't move the whole gun
	}

	if (GetAttachParent())
	{
		LastParentLocation = GetAttachParent()->GetComponentLocation();
		LastParentRotation = GetAttachParent()->GetComponentQuat();
	}
}

void UVRMechanicalComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsBeingHeld && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
	{
		UpdateFromHandLocation(DrivingGrabComponent->GetCurrentInteractor()->GetComponentLocation());
	}
	else if (!bIsBeingHeld)
	{
		CalculateInertia(DeltaTime);
		
		if (bHasReturnSpring && !bIsLocked && !FMath::IsNearlyEqual(CurrentNormalisedValue, RestingValue, 0.001f))
		{
			float SprungValue = FMath::FInterpTo(CurrentNormalisedValue, RestingValue, DeltaTime, ReturnSpeed);
			SetNormalizedValue(SprungValue);
		}
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
	float OldValue = CurrentNormalisedValue;
	CurrentNormalisedValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	
	if (OldValue != CurrentNormalisedValue)
	{
		OnValueChanged.Broadcast(CurrentNormalisedValue);
		
		// Movement Haptics
		if (bIsBeingHeld && MovementHapticEffect && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
		{
			// Pulse based on threshold
			if (FMath::Abs(CurrentNormalisedValue - LastHapticValue) >= HapticTickThreshold)
			{
				DrivingGrabComponent->GetCurrentInteractor()->PlayHapticFeedback(MovementHapticEffect, 0.3f);
				LastHapticValue = CurrentNormalisedValue;
			}
		}

		// Trigger Reached Min/Max events
		if (CurrentNormalisedValue >= 1.0f && !bWasAtMax)
		{
			OnReachedMax.Broadcast();
			bWasAtMax = true;
		}
		else if (CurrentNormalisedValue < 1.0f)
		{
			bWasAtMax = false;
		}

		if (CurrentNormalisedValue <= 0.0f && !bWasAtMin)
		{
			OnReachedMin.Broadcast();
			bWasAtMin = true;
		}
		else if (CurrentNormalisedValue > 0.0f)
		{
			bWasAtMin = false;
		}
	}

	float CurrentTargetOffset = CurrentNormalisedValue * MaxRange * (bInvertDirection ? -1.0f : 1.0f);
	
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

void UVRMechanicalComponent::SetIsLocked(bool bNewLocked)
{
	bIsLocked = bNewLocked;
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
	float CurrentRawValue = CalculateRawHandValue(HandWorldLocation);
	float DeltaRaw = 0.0f;

	if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		// FindDeltaAngleDegrees safely handles the 180/-180 wrap-around
		DeltaRaw = FMath::FindDeltaAngleDegrees(InitialGrabRawValue, CurrentRawValue);
	}
	else
	{
		DeltaRaw = CurrentRawValue - InitialGrabRawValue;
	}

	float DirectionModifier = bInvertDirection ? -1.0f : 1.0f;
	float NormalizedValue = GrabbedNormalizedValue + ((DeltaRaw / MaxRange) * DirectionModifier);
	
	SetNormalizedValue(NormalizedValue);
}

float UVRMechanicalComponent::CalculateRawHandValue(FVector HandWorldLocation) const
{
	if (!GetAttachParent()) return 0.0f;
	
	FVector HandLocalSpace = GetAttachParent()->GetComponentTransform().InverseTransformPosition(HandWorldLocation);
	FVector HandOffset = HandLocalSpace - HomeTransform.GetLocation();
	
	if (MechanicalMovementType == VRNativeTags::Linear)
	{
		return FVector::DotProduct(HandOffset, LocalAxis.GetSafeNormal());
	}
	else if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		FVector SafeAxis = LocalAxis.GetSafeNormal();
		
		// Create a consistent reference frame around the pivot axis
		FVector RefAxis = FVector::ForwardVector;
		if (FMath::Abs(FVector::DotProduct(SafeAxis, RefAxis)) > 0.9f)
		{
			RefAxis = FVector::RightVector;
		}
		
		FVector OrthogonalY = FVector::CrossProduct(SafeAxis, RefAxis).GetSafeNormal();
		FVector OrthogonalX = FVector::CrossProduct(OrthogonalY, SafeAxis).GetSafeNormal();
		
		FVector ProjectedHand = FVector::VectorPlaneProject(HandOffset, SafeAxis).GetSafeNormal();
		
		float DotX = FVector::DotProduct(ProjectedHand, OrthogonalX);
		float DotY = FVector::DotProduct(ProjectedHand, OrthogonalY);
		
		return FMath::RadiansToDegrees(FMath::Atan2(DotY, DotX));
	}
	
	return 0.0f;
}

void UVRMechanicalComponent::CalculateInertia(float DeltaTime)
{
	if (!GetAttachParent() || DeltaTime <= 0.0f) return;

	FVector CurrentLocation = GetAttachParent()->GetComponentLocation();
	FVector Velocity = (CurrentLocation - LastParentLocation) / DeltaTime;
	FVector Acceleration = (Velocity - LastParentVelocity) / DeltaTime;
	Acceleration = Acceleration.GetClampedToMaxSize(20000.0f); // Prevent VR tracking jumps
	
	LastParentLocation = CurrentLocation;
	LastParentVelocity = Velocity;
	
	FQuat CurrentRot = GetAttachParent()->GetComponentQuat();
	FQuat DeltaRot = CurrentRot * LastParentRotation.Inverse();
	LastParentRotation = CurrentRot;

	FVector Axis; float Angle;
	DeltaRot.ToAxisAndAngle(Axis, Angle);
	if (Angle > PI) Angle -= 2.0f * PI;
	
	FVector AngularVelocity = Axis * (Angle / DeltaTime);
	FVector AngularAcceleration = (AngularVelocity - LastAngularVelocity) / DeltaTime;
	AngularAcceleration = AngularAcceleration.GetClampedToMaxSize(20000.0f);
	LastAngularVelocity = AngularVelocity;

	if (bUseSimulatedInertia && !bIsBeingHeld && !bIsLocked)
	{
		float Force = 0.0f;
		FVector WorldAxis = GetAttachParent()->GetComponentTransform().TransformVectorNoScale(LocalAxis.GetSafeNormal());

		if (MechanicalMovementType == VRNativeTags::Linear)
		{
			Force = FVector::DotProduct(Acceleration, WorldAxis) * -0.0005f * InertiaMultiplier;
		}
		else if (MechanicalMovementType == VRNativeTags::Rotational)
		{
			Force = FVector::DotProduct(AngularAcceleration, WorldAxis) * -0.001f * InertiaMultiplier;
		}

		if (FMath::Abs(Force) > 0.1f) 
		{
			CurrentMomentum += Force * DeltaTime;
		}
		
		if (FMath::Abs(CurrentMomentum) > 0.001f)
		{
			float ProposedValue = CurrentNormalisedValue + (CurrentMomentum * DeltaTime);
			
			if (ProposedValue >= 1.0f || ProposedValue <= 0.0f)
			{
				CurrentMomentum = 0.0f; // Hard stop
			}
			
			SetNormalizedValue(ProposedValue);
		}
		
		CurrentMomentum = FMath::FInterpTo(CurrentMomentum, 0.0f, DeltaTime, 8.0f); // Friction
	}
	else
	{
		CurrentMomentum = 0.0f;
	}
}

void UVRMechanicalComponent::ConstructVisuals(UStaticMesh* InMesh, bool bWeldToParent)
{
	if (!InMesh) return;
	
	VisualMesh = NewObject<UStaticMeshComponent>(this, "VisualMesh");
	VisualMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	VisualMesh->SetStaticMesh(InMesh);
	
	// Mechanical parts should generally NOT be welded if they are meant to move relative to a physics-simulating root.
	// If welded, manual SetRelativeLocation calls will fight the physics simulation.
	bool bActualWeld = bWeldToParent;
	
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::KeepRelative,
		bActualWeld
		);	
	
	VisualMesh->AttachToComponent(this, AttachRules);
	VisualMesh->RegisterComponent();
	
	if (bActualWeld)
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VisualMesh->SetCollisionProfileName(TEXT("PhysicsBody"));
	}
	else
	{
		// Default to QueryOnly for moving parts to allow interaction without physics fighting.
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
}
