#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h" // Include for FGameplayTag
#include "VRWeaponData.generated.h"


UCLASS(BlueprintType, Blueprintable)
class VRMODULARWEAPONSYSTEM_API UVRWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FireRate; 
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float SpreadIntensity;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float RecoilAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	bool bUseHitscan = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	FGameplayTag CompatibleMagazinesTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
    UStaticMesh* FrameMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	UStaticMesh* BarrelMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
    UStaticMesh* MuzzleMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	UStaticMesh* TriggerMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	UStaticMesh* SliderMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	UParticleSystem* BarrelFlash;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	UParticleSystem* MuzzleFlash;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sounds")
	USoundBase* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sounds")
	USoundBase* DryFireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Sounds")
	USoundBase* ReloadSound;
	

	
#pragma region Using HitScan
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float HitscanRange = 1000.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float HitscanDamage = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	USoundBase* HitscanImpactSound;
	
	
#pragma endregion 
};