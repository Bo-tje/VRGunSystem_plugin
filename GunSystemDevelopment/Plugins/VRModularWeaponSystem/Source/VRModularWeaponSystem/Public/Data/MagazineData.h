// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MagazineData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class VRMODULARWEAPONSYSTEM_API UMagazineData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Let Unreal use the default PrimaryAssetType ("MagazineData")

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Data")
	int32 MaxAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Data")
	FGameplayTag MagazineType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Data")
	TObjectPtr<UStaticMesh> MagazineMesh;

};
