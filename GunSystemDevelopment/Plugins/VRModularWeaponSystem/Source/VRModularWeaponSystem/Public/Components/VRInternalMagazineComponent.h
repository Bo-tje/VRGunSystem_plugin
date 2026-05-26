#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interfaces/VRRoundProvider.h"
#include "VRInternalMagazineComponent.generated.h"

class USphereComponent;
class UProjectileData;
class UVRWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInternalAmmoChanged, int32, CurrentCount);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRMODULARWEAPONSYSTEM_API UVRInternalMagazineComponent : public USceneComponent, public IVRWeaponComponentInterface, public IVRRoundProvider
{
	GENERATED_BODY()

public:
	UVRInternalMagazineComponent();

	virtual void BeginPlay() override;

	// IVRWeaponComponentInterface
	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override;

	// IVRRoundProvider
	virtual bool GetRound_Implementation(UProjectileData*& OutRound) override;
	virtual bool HasRound_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Internal Magazine")
	int32 MaxCapacity = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Internal Magazine", meta = (Categories = "VRModularWeaponSystem.Ammo"))
	FGameplayTag CompatibleAmmoTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Internal Magazine", meta = (Categories = "VRModularWeaponSystem.State"))
	FGameplayTagContainer RequiredWeaponStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Internal Magazine")
	float LoadRadius = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Internal Magazine | Feedback")
	TObjectPtr<USoundBase> LoadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Internal Magazine | Feedback")
	TObjectPtr<class UHapticFeedbackEffect_Base> LoadHaptic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> LoadDetectionSphere;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Internal Magazine")
	FOnInternalAmmoChanged OnAmmoChanged;

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Internal Magazine")
	int32 GetCurrentAmmoCount() const { return LoadedRounds.Num(); }

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Internal Magazine")
	bool IsFull() const { return LoadedRounds.Num() >= MaxCapacity; }

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Internal Magazine")
	bool IsEmpty() const { return LoadedRounds.Num() == 0; }

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Weapon | Internal Magazine")
	TArray<TObjectPtr<UProjectileData>> LoadedRounds;
};
