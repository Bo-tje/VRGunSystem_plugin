// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ProjectileData.generated.h"


/**
 * Data Asset to define a specific type of ammo.
 * Designers can create new instances of this to define new ammo types without touching code.
 */
UCLASS(BlueprintType, Blueprintable)
class VRMODULARWEAPONSYSTEM_API UProjectileData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	FString ProjectileName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	float ProjectileSpeed = 40000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	float ProjectileDamage = 1.0f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	FGameplayTagContainer AmmoTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	float ProjectileGravityScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	float ProjectileMass = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Type")
	float ProjectileDragCoefficient = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Visuals")
	class UStaticMesh* ProjectileMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Visuals")
	class UStaticMesh* SpentCasingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Visuals")
	class UParticleSystem* ProjectileTrail;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Visuals")
	class UParticleSystem* ProjectileExplosion;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Sounds")
	class USoundBase* ProjectileSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo Sounds")
	class USoundBase* ProjectileImpactSound;
	
};