// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EVRInteractableAction.h"
#include "VRInteractableInterface.generated.h"

/**
 * 
 */
UINTERFACE(BlueprintType, MinimalAPI)
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
	void OnHoverStart(UObject* Interactor);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Plugin | Interaction")
	void OnHoverEnd(UObject* Interactor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Plugin | Interaction")
	void StartAction(UObject* Interactor, EVRInteractableAction Action, float Value);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "VR Plugin | Interaction")
	void StopAction(UObject* Interactor, EVRInteractableAction Action, float Value);
};
