#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/VRNativeTags.h"
#include "Data/VRWeaponData.h"
#include "Interfaces/VRWeaponInterface.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "VRMechanicalComponent.generated.h"

class UVRGrabComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMechanicalReachedTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMechanicalValueChanged, float, NewValue);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRMechanicalComponent : public USceneComponent, public IVRWeaponComponentInterface
{
	GENERATED_BODY()
public:
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> VisualMesh;
	
	UVRMechanicalComponent();
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- IVRWeaponComponentInterface ---
	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override {}
	virtual void InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings) override;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical", meta = (Categories = "VRModularWeaponSystem.MechanicalMovement"))
	FGameplayTag MechanicalMovementType;
	
	FTransform HomeTransform;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	FVector LocalAxis;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	bool bHasReturnSpring;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Mechanical")
	float ReturnSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Mechanical")
	float RestingValue = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Mechanical")
	void SetRestingValue(float NewRestingValue);

	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	TObjectPtr<UHapticFeedbackEffect_Base> MovementHapticEffect;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float HapticTickThreshold = 0.05f;
	
	UPROPERTY(BlueprintReadWrite, Category = "VR Plugin | Mechanical")
	bool bIsBeingHeld = false;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	float MaxRange;
	
	UPROPERTY(BlueprintReadOnly, Category = "VR Plugin | Mechanical")
	float CurrentNormalisedValue;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	bool bInvertDirection;

	UPROPERTY(BlueprintReadWrite, Category = "VR Plugin | Mechanical | State")
	bool bIsLocked = false;

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Mechanical | State")
	void SetIsLocked(bool bNewLocked);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Mechanical | Physics")
	bool bUseSimulatedInertia = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Mechanical | Physics", meta = (EditCondition = "bUseSimulatedInertia"))
	float InertiaMultiplier = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Mechanical | Physics")
	void AddMomentum(float MomentumAmount);

	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical | Events")
	FGameplayTag OnReachedMaxTag;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical | Events")
	FGameplayTag OnReachedMinTag;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | Mechanical")
	FOnMechanicalReachedTarget OnReachedMax;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | Mechanical")
	FOnMechanicalReachedTarget OnReachedMin;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | Mechanical")
	FOnMechanicalValueChanged OnValueChanged;

	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Mechanical")
	void SetNormalizedValue(float NewValue);
	
	UFUNCTION(BlueprintCallable, Category = "VR Plugin | Mechanical")
	void UpdateFromHandLocation(FVector HandWorldLocation);
	
	void ConstructVisuals(UStaticMesh* InMesh, bool bWeldToParent);
	
	void ApplyMechanicalSettings(UVRMechanicalSettings* Settings);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "VR Plugin | Mechanical")
	TObjectPtr<UVRGrabComponent> DrivingGrabComponent;

	UFUNCTION()
	virtual void OnGrabbed(AActor* InteractingActor);

	UFUNCTION()
	virtual void OnReleased();

	/** Updates the component's transform based on the current normalized value */
	void UpdateVisuals();

	/** Checks if we've crossed min/max thresholds and broadcasts events */
	void CheckThresholdEvents();

	/** Handles haptic pulses based on movement threshold */
	void HandleHaptics();

	/** Tracks the motion of the parent component for inertia calculations */
	void TrackParentMotion(float DeltaTime);

private:
	bool bWasAtMax = false;
	bool bWasAtMin = true;
	float LastHapticValue = 0.0f;
	
	float InitialGrabRawValue = 0.0f;
	float GrabbedNormalizedValue = 0.0f;

	struct FParentMotionData
	{
		FVector LastLocation = FVector::ZeroVector;
		FVector LastVelocity = FVector::ZeroVector;
		FQuat LastRotation = FQuat::Identity;
		FVector LastAngularVelocity = FVector::ZeroVector;
	} ParentMotion;
	
	float CurrentMomentum = 0.0f;

	float CalculateRawHandValue(FVector HandWorldLocation) const;
	void CalculateInertia(float DeltaTime);
};
