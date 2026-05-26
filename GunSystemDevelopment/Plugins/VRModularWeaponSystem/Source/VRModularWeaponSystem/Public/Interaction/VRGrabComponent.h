#pragma once

#include "CoreMinimal.h"
#include "Interfaces/VRInteractableInterface.h"
#include "Components/BoxComponent.h"
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
class VRMODULARWEAPONSYSTEM_API UVRGrabComponent : public UBoxComponent, public IVRInteractableInterface, public IVRWeaponComponentInterface
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
	TObjectPtr<UHapticFeedbackEffect_Base> GrabHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | VR Interaction | Audio")
	TObjectPtr<USoundBase> GrabSound;
	
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Visuals", meta = (Categories = "VRModularWeaponSystem.AnimPose"))
	FGameplayTag GrabPoseTag;

	/** A tag that can be read by the Interactor's Animation Blueprint to trigger a specific hand pose while hovering. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Visuals", meta = (Categories = "VRModularWeaponSystem.AnimPose"))
	FGameplayTag HoverPoseTag;

	/** Higher priority grabs take precedence when multiple components overlap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Interaction")
	int32 GrabPriority = 0;

	/** Maximum allowed distance from the surface of this box to successfully grab it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Interaction")
	float MaxGrabDistance = 12.0f;

	/** If true, this grip is considered the main handle for haptics scaling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	bool bIsMainGrip = false;

	/** Optional default name of a socket on a parent StaticMesh to use as the fallback grip anchor.
	  * If set, the weapon will snap so this socket aligns with the hand.
	  * If empty, the GrabComponent's own transform is used (current behavior). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	FName GripSocketName;

	/** Optional: Name of a socket on a parent StaticMesh to use when grabbed by the right hand.
	  * If empty, the system will fall back to GripSocketName. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	FName RightHandGripSocketName;

	/** Optional: Name of a socket on a parent StaticMesh to use when grabbed by the left hand.
	  * If empty, the system will fall back to GripSocketName. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	FName LeftHandGripSocketName;

	/** Optional default rotation offset applied after snapping. 
	  * Use this to fine-tune the weapon orientation in the hand. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	FRotator GripRotationOffset = FRotator::ZeroRotator;

	/** Optional rotation offset specifically for the right hand.
	  * If not set (remains Zero), the default GripRotationOffset is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	FRotator RightHandRotationOffset = FRotator::ZeroRotator;

	/** Optional rotation offset specifically for the left hand.
	  * If not set (remains Zero), the default GripRotationOffset is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	FRotator LeftHandRotationOffset = FRotator::ZeroRotator;

	/** If true, the weapon lerps into the hand over GrabLerpSpeed seconds instead of instant snap. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	bool bUseSmoothGrab = false;

	/** Speed of the smooth grab lerp (only used when bUseSmoothGrab is true). Higher = faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	float GrabLerpSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Setup")
	bool bShowDebugGizmos = false;

protected:
	
	UPROPERTY(BlueprintReadOnly, Category = " VR Plugin | VR Interaction")
	bool bIsHeld;

	bool bWasSimulating;
	
	UPROPERTY()
	TWeakObjectPtr<UVRInteractor> CurrentInteractor;
	
private:
	FVector LastPosition;
	TArray<FVector> VelocityBuffer;

	bool bIsLerping = false;
	FTransform LerpStartTransform;
	float LerpAlpha = 0.0f;

	FTransform GetGripAnchorTransform(EControllerHand HandSide) const;
	
	void Attach(AActor* MyOwner, UVRInteractor* TargetInteractor) const;
	void CalculateVelocity(float DeltaTime);
	void Throw(UPrimitiveComponent* RootPrim);
	void PlayHaptics() const;
};
