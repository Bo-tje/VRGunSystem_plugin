#pragma once

#include "CoreMinimal.h"
#include "VRInteractableInterface.h"
#include "Components/SceneComponent.h"
#include "VRGrabComponent.generated.h"

class UVRInteractor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabbed, AActor*, InteractingHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleased);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRGrabComponent : public USceneComponent, public IVRInteractableInterface
{
	GENERATED_BODY()

public:	
	UVRGrabComponent();
	
	UFUNCTION(BlueprintCallable, Category = "VR Plugin | VR Interaction")
	void TryGrab(UVRInteractor* Interactor);
	
	UFUNCTION(BlueprintCallable, Category = " VR Plugin | VR Interaction") 
	void TryRelease();
	
	UFUNCTION(BlueprintCallable, Category = " VR Plugin | VR Interaction")
	bool IsHeld() const {return bIsHeld; }
	
	// IVRInteractableInterface implementation
	virtual void StartAction_Implementation(UObject* Interactor) override;
	virtual void StopAction_Implementation(UObject* Interactor) override;

#pragma region events for designers to bind to
	
	UPROPERTY(BlueprintAssignable, Category = "VR Interaction")
	FOnGrabbed OnGrabbed;

	UPROPERTY(BlueprintAssignable, Category = " VR Plugin | VR Interaction")
	FOnReleased OnReleased;
	
#pragma endregion
	
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
	TWeakObjectPtr<UVRInteractor> CurrentInteractor;
	
private:
	FVector LastPosition;
	TArray<FVector> VelocityBuffer;
	
	void Attach(AActor* MyOwner, UVRInteractor* TargetInteractor) const;
	void CalculateVelocity(float DeltaTime);
	void Throw(UPrimitiveComponent* RootPrim);
	void PlayHaptics(EControllerHand Side) const;
};
