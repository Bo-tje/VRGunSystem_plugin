#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ProjectileData.generated.h"

class UNiagaraSystem;

UCLASS(BlueprintType, Blueprintable)
class VRMODULARWEAPONSYSTEM_API UProjectileData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // --- Identification ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Identity", meta = (Categories = "VRModularWeaponSystem.Ammo"))
    FGameplayTagContainer AmmoTags;

    // --- Ballistics (Physical) ---
    // The actual Actor to spawn if not using hitscan (e.g., BP_Bullet_9mm)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Physical Projectile")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Physical Projectile", meta = (Units = "cm/s"))
    float InitialSpeed = 40000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Physical Projectile")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Physical Projectile")
    float Damage = 25.0f;

    // --- Ballistics (Hitscan) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Hitscan")
    float HitscanRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Hitscan")
    float HitscanDamage = 20.0f;

    // --- Multi-Projectile (Shotgun) ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Multi-Projectile")
    int32 PelletCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Multi-Projectile")
    float SpreadAngle = 0.0f;

    // --- Visuals ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Visuals")
    TObjectPtr<UStaticMesh> LiveRoundMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Visuals")
    TObjectPtr<UStaticMesh> SpentCasingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Visuals")
    TObjectPtr<UNiagaraSystem> MuzzleFlashOverride;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Visuals")
    TObjectPtr<UNiagaraSystem> ImpactEffect;

    // --- Audio ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Audio")
    TObjectPtr<USoundBase> FireSoundOverride;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo | Audio")
    TObjectPtr<USoundBase> ImpactSound;
};