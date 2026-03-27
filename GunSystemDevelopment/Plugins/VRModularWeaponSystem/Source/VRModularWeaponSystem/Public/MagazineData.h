// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MagazineData.generated.h"

/**
 * 
 */
UCLASS()
class VRMODULARWEAPONSYSTEM_API UMagazineData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Data")
	int32 MaxAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Data")
	FGameplayTag MagazineType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Data")
	TSubclassOf<UStaticMesh> MagazineMesh;

};
