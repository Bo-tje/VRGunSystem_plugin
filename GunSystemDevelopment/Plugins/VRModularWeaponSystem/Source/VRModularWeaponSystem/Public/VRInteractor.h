// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRInteractorInterface.h"
#include "Components/SceneComponent.h"
#include "VRInteractor.generated.h"

class UHapticFeedbackEffect_Base;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRInteractor : public USceneComponent, public IVRInteractorInterface
{
	GENERATED_BODY()

public:	
	UVRInteractor();
	
	
#pragma region  events callable by designers
	
	UFUNCTION(BlueprintCallable, Category= "VR Plugin | Input")
	void IntendGrab();
 	
	UFUNCTION(BlueprintCallable, Category= "VR Plugin | Input")
	void IntendRelease();

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Input")
	void IntendActionStart(float ActionValue, EVRInteractableActions ActionType);

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Input")
	void IntendActionStop(EVRInteractableActions ActionType);

#pragma endregion 

	// Called by a GrabComponent if another hand wants to grab the holding object
	void RequestRelease();
	
	// In VRInteractor.h so that we easily give the grab component a lot of information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Input")
	EControllerHand HandSide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Haptics")
	UHapticFeedbackEffect_Base* HoverHapticEffect;

	virtual APlayerController* GetProvidingPlayerController() const override;
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Components") 
	class USphereComponent* DetectionSphereComponent;
	
	UPROPERTY()
	class UVRGrabComponent* ActiveGrabComponent;
	
	UPROPERTY()
	TWeakObjectPtr<UVRGrabComponent> HoverTarget;
	
	// function to find the best grab target in range if there are multiple found
	UVRGrabComponent* GetBestGrabTarget() const;
	void UpdateBestHoverTarget();
	
	FTimerHandle HoverTimerHandle;
	
	
	// A list of all grabbable components found by the detection sphere
	UPROPERTY()
	TArray<TWeakObjectPtr<UVRGrabComponent>> OverlappingGrabs;

	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
		
};
