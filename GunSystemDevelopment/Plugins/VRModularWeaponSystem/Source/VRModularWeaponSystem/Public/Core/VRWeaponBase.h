#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/VRWeaponInterface.h"
#include "Interfaces/VRInteractableInterface.h"
#include "GameplayTagContainer.h"
#include "Core/VRNativeTags.h"
#include "Components/StateTreeComponent.h"
#include "Data/VRWeaponData.h"
#include "VRWeaponBase.generated.h"

class UStaticMeshComponent;
class UVRGrabComponent;

/**
 * AVRWeaponBase is a container actor that holds modular components together.
 */
UCLASS(Abstract, Blueprintable)
class VRMODULARWEAPONSYSTEM_API AVRWeaponBase : public AActor, public IVRWeaponInterface, public IVRInteractableInterface
{
	GENERATED_BODY()

public:
	AVRWeaponBase();

	/** Optimized Factory: Spawns a weapon and injects DataAsset before construction. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Factory", meta = (WorldContext = "WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static AVRWeaponBase* SpawnWeaponFromData(const UObject* WorldContextObject, UVRWeaponData* InData, FTransform SpawnTransform, TSubclassOf<AVRWeaponBase> WeaponClass);

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	class UBoxComponent* WeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Parts")
	USceneComponent* PartRoot;
	
	/** Components that need initialization when WeaponData changes. */
	UPROPERTY(Transient)
	TArray<UActorComponent*> CachedWeaponComponents;

	/** Components that receive input events (PullTrigger, etc). */
	UPROPERTY(Transient)
	TArray<UActorComponent*> CachedInputComponents;

	/** Map of dynamically spawned components by their name for Blueprint access. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "VR Weapon | Parts")
	TMap<FName, UActorComponent*> DynamicComponentsMap;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Logic")
	TObjectPtr<class UVRWeaponStateTreeComponent> StateTreeComponent;

	// --- Data ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Config")
	TObjectPtr<UVRWeaponData> WeaponData;

	// --- IVRWeaponInterface ---
	virtual void PullTrigger_Implementation() override;
	virtual void ReleaseTrigger_Implementation() override;
	virtual void PrimaryAction_Implementation() override;
	virtual void ReleasePrimaryAction_Implementation() override;
	virtual void SecondaryAction_Implementation() override;
	virtual void ReleaseSecondaryAction_Implementation() override;
	virtual bool IsTriggerPulled_Implementation() const override;

	// --- IVRInteractableInterface ---

	virtual void StartAction_Implementation(UObject* Interactor, float ActionValue, FGameplayTag ActionTag) override;
	virtual void StopAction_Implementation(UObject* Interactor, FGameplayTag ActionTag) override;
	virtual void OnHoverStart_Implementation(UObject* Interactor) override {}
	virtual void OnHoverEnd_Implementation(UObject* Interactor) override {}

	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;

	/** Spawns and attaches meshes defined in WeaponData. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon")
	void ApplyWeaponDataVisuals();

	/** Distributes WeaponData to all components. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon")
	void InitializeWeapon();

	UPROPERTY(BlueprintReadOnly, Category = "VR Weapon | Interaction")
	int32 ActiveGrabCount = 0;

	UPROPERTY(Transient)
	TArray<UVRGrabComponent*> CachedGrabComponents;

public:

	UPROPERTY(Transient, BlueprintReadOnly, Category = "VR Weapon | Parts")
	TArray<UActorComponent*> CachedRoundProviders;

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Interaction")
	UActorComponent* GetDynamicComponentByName(FName ComponentName) const;

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Interaction")
	class UVRInteractor* GetHoldingInteractor() const;

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Interaction")
	TArray<class UVRInteractor*> GetHoldingInteractors() const;

	// --- Interaction Callbacks ---

	UPROPERTY(BlueprintReadOnly, Category = "VR Weapon | Interaction")
	bool bIsTriggerPulled = false;

	UFUNCTION()
	virtual void OnGrabbed(AActor* InteractingHand);

	UFUNCTION()
	virtual void OnReleased();

private:
};
