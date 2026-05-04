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
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	float ReturnSpeed = 15.0f;
	
	UPROPERTY(BlueprintReadWrite, Category = "VR Plugin | Mechanical")
	bool bIsBeingHeld = false;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	float MaxRange;
	
	UPROPERTY(BlueprintReadOnly, Category = "VR Plugin | Mechanical")
	float CurrentNormalisedValue;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical")
	bool bInvertDirection;

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
	void OnGrabbed(AActor* Interactor);

	UFUNCTION()
	void OnReleased();

private:
	bool bWasAtMax = false;
	bool bWasAtMin = true;
};
