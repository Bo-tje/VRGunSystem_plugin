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
	RoundVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoundVisualMesh"));
	RoundVisualMesh->SetupAttachment(this);
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
	// If it's already empty, do nothing
	if (IsEmpty() && !LoadedProjectile) return nullptr;

	UProjectileData* EjectedRound = LoadedProjectile;
	
	if (EjectedRound)
	{
		OnRoundEjected.Broadcast(EjectedRound);
	}
	
	// Ensure everything is cleared
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
		if (LoadedProjectile)
		{
			if (CurrentChamberState == VRNativeTags::Chamber_SpentCasing && LoadedProjectile->SpentCasingMesh)
			{
				RoundVisualMesh->SetStaticMesh(LoadedProjectile->SpentCasingMesh);
			}
			else if (LoadedProjectile->LiveRoundMesh)
			{
				RoundVisualMesh->SetStaticMesh(LoadedProjectile->LiveRoundMesh);
			}
		}
	}
}
