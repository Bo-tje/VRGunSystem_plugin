#include "Data/VRWeaponData.h"
#include "Components/VRMechanicalComponent.h"

UVRMechanicalComponent::UVRMechanicalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVRMechanicalComponent::BeginPlay()
{
	Super::BeginPlay();
	HomeTransform = GetRelativeTransform();
}


void UVRMechanicalComponent::SetNormalizedValue(float NewValue)
{
	CurrentNormalisedValue = FMath::Clamp(NewValue, 0.0f, 1.0f);
	float CurrentTargetLocation = CurrentNormalisedValue * MaxRange * (bInvertDirection ? -1.0f : 1.0f);
	
	
	
	if (MechanicalMovementType == VRNativeTags::Linear)
	{
		FVector NewLocation = HomeTransform.GetLocation() + LocalAxis * CurrentTargetLocation;
		SetRelativeLocation(NewLocation);
	}

	else if (MechanicalMovementType == VRNativeTags::Rotational)
	{
		FQuat NewRotation = HomeTransform.GetRotation() * FQuat(LocalAxis, FMath::DegreesToRadians(CurrentTargetLocation));
		SetRelativeRotation(NewRotation);
	}
}

void UVRMechanicalComponent::UpdateFromHandLocation(FVector HandWorldLocation)
{
	if (!GetAttachParent()) return;
	
	FVector HandLocalSpace = GetAttachParent()->GetComponentTransform().InverseTransformPosition(HandWorldLocation);
	
	FVector HandOffset = HandLocalSpace - HomeTransform.GetLocation();
	
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
}

void UVRMechanicalComponent::ApplyMechanicalSettings(UVRMechanicalSettings* Settings)
{
	if (!Settings) return;
    
	MechanicalMovementType = Settings->MechanicalMovementType;
	LocalAxis = Settings->LocalAxis;
	MaxRange = Settings->MaxRange;
	bInvertDirection = Settings->bInvertDirection;
}


