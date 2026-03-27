// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRInteractableInterface.generated.h"

/**
 * 
 */
UINTERFACE(MinimalAPI)
class UVRInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class IVRInteractableInterface
{
	GENERATED_BODY()

	
public:
	// Add interface functions 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Plugin | Interaction")
	void OnHoverStart(USceneComponent* CallingComponent);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Plugin | Interaction")
	void OnHoverEnd(USceneComponent* CallingComponent);
};
