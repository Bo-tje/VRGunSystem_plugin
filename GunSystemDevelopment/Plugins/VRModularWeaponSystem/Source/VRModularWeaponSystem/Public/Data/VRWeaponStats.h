#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VRWeaponStats.generated.h"

/**
 * Basic weapon stats structure.
 */
USTRUCT(BlueprintType)
struct FVRWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float FireRate = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float RecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BulletVelocityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ReloadSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Spread")
	float SpreadMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Spread")
	int32 PelletCountOffset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilPitch = -1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilYaw = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilSpringStiffness = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilSpringDamping = 4.0f;


	// Overrides
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<UParticleSystem> MuzzleFlashOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TObjectPtr<USoundBase> FireSoundOverride = nullptr;
};

/**
 * A data asset containing stat modifiers, used by attachments.
 */
UCLASS(BlueprintType)
class VRMODULARWEAPONSYSTEM_API UVRWeaponStatModifier : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Flat additions to stats. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
	float FireRateOffset = 0.0f;

	/** Multipliers for stats (1.0 = no change). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
	float RecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
	float BulletVelocityMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers")
	float ReloadSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers|Spread")
	float SpreadMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifiers|Spread")
	int32 PelletCountOffset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilPitch = -1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilYaw = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilSpringStiffness = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Recoil")
	float RecoilSpringDamping = 4.0f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UParticleSystem> MuzzleFlashOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<USoundBase> FireSoundOverride;
};
