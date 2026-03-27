// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRInteractorInterface.h"
#include "Components/SceneComponent.h"
#include "VRInteractor.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRInteractor : public USceneComponent, public IVRInteractorInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVRInteractor();
	
	
	// The "Trigger" or "Grip" input will call these functions
	UFUNCTION(BlueprintCallable, Category= "VR Plugin | Input")
	void IntendGrab();
 	
	UFUNCTION(BlueprintCallable, Category= "VR Plugin | Input")
	void IntendRelease();
	
	// In VRInteractor.h
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Input")
	EControllerHand HandSide;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// The sphere component used for detecting interactable objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Components") 
	class USphereComponent* DetectionSphereComponent;

	// track what is currently being held
	UPROPERTY()
	class UVRGrabComponent* ActiveGrabComponent;
	
	// The component we are currently highlighting
	UPROPERTY()
	TWeakObjectPtr<UVRGrabComponent> HoverTarget;
	
	// helper function to find the best grab target in range if there are multiple found
	UVRGrabComponent* GetBestGrabTarget() const;
	
	void UpdateBestHoverTarget();
	
	FTimerHandle HoverTimerHandle;
	
	
	// A list of all grabbable components currently within range
	UPROPERTY()
	TArray<TWeakObjectPtr<UVRGrabComponent>> OverlappingGrabs;

	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	virtual APlayerController* GetProvidingPlayerController() const override;
		
};
