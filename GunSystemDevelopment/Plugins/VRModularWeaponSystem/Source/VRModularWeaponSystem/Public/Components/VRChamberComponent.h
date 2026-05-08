#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interfaces/VRRoundProvider.h"
#include "Data/VRWeaponData.h"
#include "VRChamberComponent.generated.h"

class UProjectileData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChamberStateChanged, FGameplayTag, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundLoaded, UProjectileData*, LoadedRound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundFired, UProjectileData*, FiredRound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundEjected, UProjectileData*, EjectedRound);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRMODULARWEAPONSYSTEM_API UVRChamberComponent : public USceneComponent, public IVRWeaponComponentInterface, public IVRRoundProvider
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	UVRChamberComponent();

	// --- IVRRoundProvider ---
	virtual bool GetRound_Implementation(UProjectileData*& OutRound) override;

	// --- IVRWeaponComponentInterface ---
	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override;

#pragma region events for designers to bind to

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Chamber")
	FOnChamberStateChanged OnChamberStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Chamber")
	FOnRoundLoaded OnRoundLoaded;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Chamber")
	FOnRoundFired OnRoundFired;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Chamber")
	FOnRoundEjected OnRoundEjected;
	
#pragma endregion 
	
#pragma region Actions
	
	/** Used by the FireComponent to take a bullet for firing. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Chamber")
	bool TryGiveBullet();

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Chamber")
	bool TryLoad(UProjectileData* NewRound);

	/** Ejects whatever is currently in the chamber. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Chamber")
	UProjectileData* TryEject();
	
#pragma endregion 
	
	UFUNCTION(BlueprintPure, Category = "VR Weapon | Chamber")
	bool IsRoundReady() const;
	
	UFUNCTION(BlueprintPure, Category = "VR Weapon | Chamber")
	bool IsEmpty() const;

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Chamber")
	FGameplayTag GetChamberState() const { return CurrentChamberState; }

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Chamber")
	UProjectileData* GetLoadedRound() const { return LoadedProjectile; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Visuals")
	UStaticMeshComponent* RoundVisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Config")
	float JamChance = 0.05f;
	
	void SetChamberState(FGameplayTag NewState);

protected:
	
	
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Chamber | Visuals")
	void UpdateVisuals();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Weapon | Chamber")
	FGameplayTag CurrentChamberState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Weapon | Chamber")
	UProjectileData* LoadedProjectile;

	UPROPERTY(BlueprintReadOnly, Category = "VR Weapon | Chamber")
	TObjectPtr<UVRWeaponData> WeaponData;
};
