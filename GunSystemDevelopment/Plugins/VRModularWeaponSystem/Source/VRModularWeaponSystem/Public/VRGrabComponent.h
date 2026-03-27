// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VRGrabComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabbed, AActor*, InteractingHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleased);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVRGrabComponent();
	
	// Change this:
	// void TryGrab(USceneComponent* TargetComponent, EControllerHand Side);

	// To this:
	UFUNCTION(BlueprintCallable, Category = "VR Plugin | VR Interaction")
	void TryGrab(UVRInteractor* Interactor);
	
	UFUNCTION(BlueprintCallable, Category = " VR Plugin | VR Interaction") 
	void TryRelease();
	
	UFUNCTION(BlueprintCallable, Category = " VR Plugin | VR Interaction")
	bool IsHeld() const {return bIsHeld; }
	
	//events for designers to bind to
	UPROPERTY(BlueprintAssignable, Category = "VR Interaction")
	FOnGrabbed OnGrabbed;

	UPROPERTY(BlueprintAssignable, Category = " VR Plugin | VR Interaction")
	FOnReleased OnReleased;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = " VR Plugin | VR Interaction")
	UHapticFeedbackEffect_Base* GrabHapticEffect;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | VR Interaction")
	float HapticScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = " VR Plugin | VR Interaction")
	bool bLoopHaptics = false;
	
	UPROPERTY(EditAnywhere, Category = " VR Plugin | VR Interaction")
	float ThrowMultiplier = 1.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	bool bUseSocketSnap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup", meta = (EditCondition = "bUseSocketSnap"))
	FName GrabSocketName = FName("GripSocket");
	
protected:
	
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintReadOnly, Category = " VR Plugin | VR Interaction")
	bool bIsHeld;

	bool bWasSimulating;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> HoldingHand;
	
private:
	FVector LastPosition;
	FVector CurrentVelocity;
	TArray<FVector> VelocityBuffer;
	
	void Attach(AActor* MyOwner, USceneComponent* TargetComponent) const;
	void CalculateVelocity(float DeltaTime);
	void Throw(UPrimitiveComponent* RootPrim);
	void PlayHaptics(EControllerHand Side) const;
};
