#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRWeaponInterface.h"
#include "VRWeaponBase.generated.h"

class UVRWeaponData;
class UStaticMeshComponent;

/**
 * AVRWeaponBase is a container actor that holds modular components together.
 * It distributes VRWeaponData to all its components and manages the physical meshes.
 */
UCLASS(Abstract, Blueprintable)
class VRMODULARWEAPONSYSTEM_API AVRWeaponBase : public AActor, public IVRWeaponInterface
{
	GENERATED_BODY()

public:
	AVRWeaponBase();

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	USceneComponent* WeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	UStaticMeshComponent* FrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	UStaticMeshComponent* BarrelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	UStaticMeshComponent* MuzzleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	UStaticMeshComponent* TriggerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	UStaticMeshComponent* SliderMesh;

	// --- Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Config")
	UVRWeaponData* WeaponData;

	// --- IVRWeaponInterface ---
	virtual void PullTrigger_Implementation() override;
	virtual void ReleaseTrigger_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Distributes WeaponData to all components that implement IVRWeaponComponentInterface. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon")
	void InitializeWeapon();

	/** Updates the static meshes based on the WeaponData asset. */
	void ApplyWeaponDataVisuals();
};
