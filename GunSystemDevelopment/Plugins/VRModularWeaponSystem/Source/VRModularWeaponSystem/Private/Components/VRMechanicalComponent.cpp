#include "Components/VRMechanicalComponent.h"
#include "Data/VRWeaponData.h"
#include "Interaction/VRGrabComponent.h"
#include "Interaction/VRInteractor.h"
#include "Components/VRWeaponStateTreeComponent.h"

UVRMechanicalComponent::UVRMechanicalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	MechanicalMovementType = VRNativeTags::Linear;
	LocalAxis = FVector::ForwardVector;
	MaxRange = 10.0f;
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
}

void UVRMechanicalComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsBeingHeld && DrivingGrabComponent && DrivingGrabComponent->GetCurrentInteractor())
	{
		UpdateFromHandLocation(DrivingGrabComponent->GetCurrentInteractor()->GetComponentLocation());
	}
	else if (!bIsBeingHeld && bHasReturnSpring && CurrentNormalisedValue > 0.0f)
	{
		float SprungValue = FMath::FInterpTo(CurrentNormalisedValue, 0.0f, DeltaTime, ReturnSpeed);
		SetNormalizedValue(SprungValue);
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
		
		// Trigger Reached Min/Max events
		if (CurrentNormalisedValue >= 1.0f && !bWasAtMax)
		{
			OnReachedMax.Broadcast();
			bWasAtMax = true;
			
			if (OnReachedMaxTag.IsValid())
			{
				if (AActor* Owner = GetOwner())
				{
					if (UVRWeaponStateTreeComponent* ST = Owner->FindComponentByClass<UVRWeaponStateTreeComponent>())
					{
						ST->SendStateTreeEvent(OnReachedMaxTag);
					}
				}
			}
		}
		else if (CurrentNormalisedValue < 1.0f)
		{
			bWasAtMax = false;
		}

		if (CurrentNormalisedValue <= 0.0f && !bWasAtMin)
		{
			OnReachedMin.Broadcast();
			bWasAtMin = true;
			
			if (OnReachedMinTag.IsValid())
			{
				if (AActor* Owner = GetOwner())
				{
					if (UVRWeaponStateTreeComponent* ST = Owner->FindComponentByClass<UVRWeaponStateTreeComponent>())
					{
						ST->SendStateTreeEvent(OnReachedMinTag);
					}
				}
			}
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

void UVRMechanicalComponent::UpdateFromHandLocation(FVector HandWorldLocation)
{
	if (!GetAttachParent()) return;
	
	// Get hand location in parent space
	FVector HandLocalSpace = GetAttachParent()->GetComponentTransform().InverseTransformPosition(HandWorldLocation);
	
	// Vector from home to hand
	FVector HandOffset = HandLocalSpace - HomeTransform.GetLocation();
	
	// Project onto movement axis
	float DotProduct = FVector::DotProduct(HandOffset, LocalAxis.GetSafeNormal());
	
	float NormalizedValue = DotProduct / MaxRange;
	
	SetNormalizedValue(NormalizedValue);
}

void UVRMechanicalComponent::ConstructVisuals(UStaticMesh* InMesh, bool bWeldToParent)
{
	if (!InMesh) return;
	
	VisualMesh = NewObject<UStaticMeshComponent>(this, "VisualMesh");
	VisualMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
	VisualMesh->SetStaticMesh(InMesh);
	
	FAttachmentTransformRules AttachRules(
		EAttachmentRule::KeepRelative,
		bWeldToParent
		);	
	
	VisualMesh->AttachToComponent(this, AttachRules);
	VisualMesh->RegisterComponent();
	
	if (bWeldToParent)
	{
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		VisualMesh->SetCollisionProfileName(TEXT("PhysicsBody"));
	}
	else
	{
		// CRITICAL FIX: Unwelded parts MUST ignore physics, otherwise they violently push the gun away!
		VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		VisualMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
}

void UVRMechanicalComponent::ApplyMechanicalSettings(UVRMechanicalSettings* Settings)
{
	if (!Settings) return;
    
	MechanicalMovementType = Settings->MechanicalMovementType;
	LocalAxis = Settings->LocalAxis;
	MaxRange = Settings->MaxRange;
	bInvertDirection = Settings->bInvertDirection;
	
	
	bHasReturnSpring = Settings->bHasReturnSpring;
	ReturnSpeed = Settings->ReturnSpeed;
	OnReachedMaxTag = Settings->OnReachedMaxTag;
	OnReachedMinTag = Settings->OnReachedMinTag;
}

void UVRMechanicalComponent::OnGrabbed(AActor* Interactor)
{
	bIsBeingHeld = true;
}

void UVRMechanicalComponent::OnReleased()
{
	bIsBeingHeld = false;
}
