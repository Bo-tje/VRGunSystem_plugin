#pragma once

#include "CoreMinimal.h"
#include "Interfaces/VRInteractableInterface.h"
#include "Components/SphereComponent.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "VRGrabComponent.generated.h"

class UVRInteractor;
class UVRWeaponData;
class UVRWeaponComponentSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoverStart, UObject*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHoverEnd, UObject*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStartAction, UObject*, Interactor, float, Value,  FGameplayTag, ActionTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStopAction, UObject*, Interactor, FGameplayTag, ActionTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputActionValue, UObject*, Interactor, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputAction, UObject*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabbed, AActor*, InteractingHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrabReleased);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRGrabComponent : public USphereComponent, public IVRInteractableInterface, public IVRWeaponComponentInterface
{
	GENERATED_BODY()

public:	
	UVRGrabComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable, Category = "VR Plugin | VR Interaction")
	void TryGrab(UVRInteractor* Interactor);
	
	UFUNCTION(BlueprintCallable, Category = "VR Plugin | VR Interaction")
	void TryRelease();
	
	UFUNCTION(BlueprintCallable, Category = " VR Plugin | VR Interaction")
	bool IsHeld() const {return bIsHeld; }

	UFUNCTION(BlueprintPure, Category = "VR Plugin | VR Interaction")
	EControllerHand GetHoldingHand() const;
	
	UFUNCTION(BlueprintPure, Category = "VR Plugin | VR Interaction")
	UVRInteractor* GetCurrentInteractor() const { return CurrentInteractor.Get(); }
	

	
	// IVRInteractableInterface implementation
	virtual void OnHoverStart_Implementation(UObject* Interactor) override;
	virtual void OnHoverEnd_Implementation(UObject* Interactor) override;
	virtual void StartAction_Implementation(UObject* Interactor, float ActionValue, FGameplayTag ActionTag) override;
	virtual void StopAction_Implementation(UObject* Interactor, FGameplayTag ActionTag) override;

	// IVRWeaponComponentInterface implementation
	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override;
	virtual void InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings) override;

#pragma region events for designers to bind to
	
	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction")
	FOnHoverStart OnHoverStart;
	
	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction")
	FOnHoverEnd OnHoverEnd;
	
	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction")
	FStartAction StartAction;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction")
	FStopAction StopAction;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction | Common Inputs")
	FOnInputActionValue OnTriggerStart;
	
	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction | Common Inputs")
	FOnInputAction OnTriggerStop;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction | Common Inputs")
	FOnInputActionValue OnPrimaryActionStart;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction | Common Inputs")
	FOnInputAction OnPrimaryActionStop;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction | Common Inputs")
	FOnInputActionValue OnSecondaryActionStart;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction | Common Inputs")
	FOnInputAction OnSecondaryActionStop;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction")
	FOnGrabbed OnGrabbed;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | VR Interaction")
	FOnGrabReleased OnGrabReleased;
	
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
	bool bSnapToInteractor = true;

	/** If true, the entire owner actor will be attached to the interactor. Set to false for sub-components like slides. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	bool bAttachOwnerOnGrab = true;

	/** Distance at which the grab will automatically break. Used for sub-components like slides. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Interaction")
	float BreakDistance = 25.0f;
	
	/** A tag that can be read by the Interactor's Animation Blueprint to trigger a specific hand pose. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Visuals")
	FGameplayTag GrabPoseTag;

	/** A tag that can be read by the Interactor's Animation Blueprint to trigger a specific hand pose while hovering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Visuals")
	FGameplayTag HoverPoseTag;

	/** Higher priority grabs take precedence when multiple components overlap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Interaction")
	int32 GrabPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Plugin | Interaction")
	bool bUseBoxCollision = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Plugin | Interaction", meta = (EditCondition = "bUseBoxCollision"))
	FVector BoxExtents = FVector(10.0f, 10.0f, 10.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Plugin | Components")
	class UBoxComponent* BoxCollider;

	/** If true, this grip is considered the main handle for haptics scaling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	bool bIsMainGrip = false;

protected:
	
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
	void PlayHaptics() const;
};
