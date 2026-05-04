#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/VRNativeTags.h"
#include "VRMechanicalComponent.generated.h"
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRMechanicalComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	
	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> VisualMesh;
	
	
	UVRMechanicalComponent();
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "VR Plugin | Mechanical", meta = (Categories = "VRModularWeaponSystem.MechanicalMovement"))
	FGameplayTag MechanicalMovementType;
	
	FTransform HomeTransform;
	
	UPROPERTY(EditAnywhere)
	FVector LocalAxis;
	
	
	UPROPERTY(EditAnywhere)
	float MaxRange;
	
	float CurrentNormalisedValue;
	
	UPROPERTY(EditAnywhere)
	bool bInvertDirection;

	UFUNCTION()
	void SetNormalizedValue(float NewValue);
	
	UFUNCTION()
	void UpdateFromHandLocation(FVector HandWorldLocation);
	
	void ConstructVisuals(UStaticMesh* InMesh, bool bWeldToParent);
	
	void ApplyMechanicalSettings(UVRMechanicalSettings* Settings);
};
