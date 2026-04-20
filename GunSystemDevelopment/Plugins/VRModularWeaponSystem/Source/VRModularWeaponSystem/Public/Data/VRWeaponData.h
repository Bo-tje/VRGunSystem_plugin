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
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Sockets")
	FName ParentSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Sockets")
	FTransform PartOffset;
};

USTRUCT(BlueprintType)
struct FVRWeaponDynamicComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Sockets")
	FName ParentSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Sockets")
	FTransform RelativeOffset;
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
	float FireRate = 600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float RecoilAmount = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FGameplayTag CompatibleMagazinesTag;

	// --- Modular Parts ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Composition")
	TArray<FVRWeaponPart> WeaponParts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Composition")
	TArray<FVRWeaponDynamicComponent> AdditionalComponents;

	/** The projectile this weapon fires. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Specs")
	TObjectPtr<UProjectileData> DefaultProjectile;

	// --- Visual FX ---
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> DryFireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> ReloadSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHapticEffect;
	
};

