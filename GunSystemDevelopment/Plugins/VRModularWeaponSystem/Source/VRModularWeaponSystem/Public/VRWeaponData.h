#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "VRWeaponData.generated.h"

class UProjectileData;

USTRUCT(BlueprintType)
struct FVRWeaponPart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FName PartName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	UStaticMesh* Mesh;
};

/**
 * UVRWeaponData defines the configuration for a weapon.
 */
UCLASS(BlueprintType, Blueprintable)
class VRMODULARWEAPONSYSTEM_API UVRWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	FString WeaponName;

	// --- Base Stats ---
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	bool bUseHitscan = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float HitscanDamage = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float HitscanRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FireRate = 600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float RecoilAmount = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FGameplayTag CompatibleMagazinesTag;

	// --- Modular Parts ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Composition")
	TArray<FVRWeaponPart> WeaponParts;

	/** The projectile this weapon fires. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Specs")
	UProjectileData* DefaultProjectile;

	// --- Visual FX ---
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	USoundBase* DryFireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	USoundBase* HitscanImpactSound;
};

