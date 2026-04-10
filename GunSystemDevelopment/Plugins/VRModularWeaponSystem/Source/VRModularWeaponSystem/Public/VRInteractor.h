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
	void IntendActionStart(float ActionValue, UPARAM(meta = (Categories = "VRModularWeaponSystem.Interaction")) FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Input")
	void IntendActionStop(UPARAM(meta = (Categories = "VRModularWeaponSystem.Interaction")) FGameplayTag ActionTag);

#pragma endregion 
	
	void RequestRelease();
	
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
	
	UVRGrabComponent* GetBestGrabTarget() const;
	void UpdateBestHoverTarget();
	
	FTimerHandle HoverTimerHandle;
	
	
	UPROPERTY()
	TArray<TWeakObjectPtr<UVRGrabComponent>> OverlappingGrabs;

	UFUNCTION()
	void OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
		
};
