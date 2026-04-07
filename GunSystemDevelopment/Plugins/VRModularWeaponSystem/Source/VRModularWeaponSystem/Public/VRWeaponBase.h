#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRWeaponInterface.h"
#include "GameplayTagContainer.h"
#include "VRWeaponBase.generated.h"

class UVRWeaponData;
class UStaticMeshComponent;
class UVRGrabComponent;

/**
 * AVRWeaponBase is a container actor that holds modular components together.
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
	USceneComponent* PartRoot;

	/** Tracks all components created in the Construction Script so we can clean them up properly. */
	UPROPERTY(Transient)
	TArray<UActorComponent*> DynamicComponents;

	// --- Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Config")
	UVRWeaponData* WeaponData;

	// --- IVRWeaponInterface ---
	virtual void PullTrigger_Implementation() override;
	virtual void ReleaseTrigger_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Spawns and attaches meshes defined in WeaponData. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon")
	void ApplyWeaponDataVisuals();

	/** Distributes WeaponData to all components. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon")
	void InitializeWeapon();

	// --- Input Handling ---

	/** Reference to the grab component on this weapon. */
	UPROPERTY(BlueprintReadOnly, Category = "VR Weapon | Interaction")
	UVRGrabComponent* GrabComponent;

	UFUNCTION()
	virtual void OnGrabbed(AActor* InteractingHand);

	UFUNCTION()
	virtual void OnReleased();

	UFUNCTION()
	virtual void HandleActionStart(UObject* Interactor, float Value, FGameplayTag ActionTag);

	UFUNCTION()
	virtual void HandleActionStop(UObject* Interactor, FGameplayTag ActionTag);

private:
	void ClearOldParts();
};
