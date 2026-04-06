#include "VRChamberComponent.h"
#include "VRNativeTags.h"
#include "ProjectileData.h"
#include "VRWeaponData.h"
#include "Components/StaticMeshComponent.h"

UVRChamberComponent::UVRChamberComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentChamberState = VRNativeTags::Chamber_Empty;
	LoadedProjectile = nullptr;
	RoundVisualMesh = nullptr;
}

void UVRChamberComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	WeaponData = InData;
}

bool UVRChamberComponent::GetRound_Implementation(UProjectileData*& OutRound)
{
	OutRound = LoadedProjectile;
	return TryGiveBullet();
}

void UVRChamberComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateVisuals();
}

bool UVRChamberComponent::TryGiveBullet()
{
	if (IsRoundReady())
	{
		OnRoundFired.Broadcast(LoadedProjectile);
		
		SetChamberState(VRNativeTags::Chamber_SpentCasing);
		return true;
	}
	return false;
}

bool UVRChamberComponent::TryLoad(UProjectileData* NewRound)
{
	if (!NewRound) return false;

	if (IsEmpty())
	{
		LoadedProjectile = NewRound;
		SetChamberState(VRNativeTags::Chamber_RoundReady);
		OnRoundLoaded.Broadcast(LoadedProjectile);
		return true;
	}

	return false;
}

UProjectileData* UVRChamberComponent::TryEject()
{
	if (IsEmpty()) return nullptr;

	UProjectileData* EjectedRound = LoadedProjectile;
	OnRoundEjected.Broadcast(EjectedRound);
	
	LoadedProjectile = nullptr;
	SetChamberState(VRNativeTags::Chamber_Empty);

	return EjectedRound;
}

bool UVRChamberComponent::IsRoundReady() const
{
	return CurrentChamberState == VRNativeTags::Chamber_RoundReady && LoadedProjectile != nullptr;
}

bool UVRChamberComponent::IsEmpty() const
{
	return CurrentChamberState == VRNativeTags::Chamber_Empty;
}

void UVRChamberComponent::SetChamberState(FGameplayTag NewState)
{
	if (CurrentChamberState != NewState)
	{
		CurrentChamberState = NewState;
		UpdateVisuals();
		OnChamberStateChanged.Broadcast(CurrentChamberState);
	}
}

void UVRChamberComponent::UpdateVisuals()
{
	if (!RoundVisualMesh) return;

	if (IsEmpty())
	{
		RoundVisualMesh->SetVisibility(false);
	}
	else
	{
		RoundVisualMesh->SetVisibility(true);
		if (LoadedProjectile && LoadedProjectile->ProjectileMesh)
		{
			RoundVisualMesh->SetStaticMesh(LoadedProjectile->ProjectileMesh);
		}
	}
}
