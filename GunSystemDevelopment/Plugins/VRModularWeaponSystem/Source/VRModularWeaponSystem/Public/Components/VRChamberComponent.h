#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interfaces/VRRoundProvider.h"
#include "Data/VRWeaponData.h"
#include "VRChamberComponent.generated.h"

class UProjectileData;
class UNiagaraSystem;

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
	virtual bool HasRound_Implementation() const override;

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
	TObjectPtr<UStaticMeshComponent> RoundVisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Config")
	float JamChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Ejection")
	TSubclassOf<class AVREjectedCasing> EjectedCasingClass;

	/** The direction vector in local chamber space to eject the casing/round. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Ejection")
	FVector EjectVelocityDirection = FVector(150.0f, 50.0f, 50.0f);

	/** The strength or speed multiplier of the ejection impulse. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Ejection")
	float EjectVelocityStrength = 1.0f;

	/** Optional bounce sounds override for physical casings ejected from this chamber. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Ejection")
	TArray<TObjectPtr<USoundBase>> BounceSoundsOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Manual Loading")
	bool bAllowManualLoading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Manual Loading", meta = (Categories = "VRModularWeaponSystem.Ammo"))
	FGameplayTag CompatibleAmmoTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Manual Loading", meta = (Categories = "VRModularWeaponSystem.State"))
	FGameplayTagContainer RequiredWeaponStateTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Manual Loading")
	float ManualLoadRadius = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Manual Loading")
	TObjectPtr<USoundBase> ManualLoadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Manual Loading")
	TObjectPtr<class UHapticFeedbackEffect_Base> ManualLoadHaptic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> LoadDetectionSphere;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Smoke")
	TObjectPtr<UNiagaraSystem> ChamberSmokeNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Smoke")
	FVector ChamberSmokeOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Chamber | Smoke")
	FRotator ChamberSmokeRotation = FRotator::ZeroRotator;

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Chamber | Smoke")
	void SpawnChamberSmoke();

	UFUNCTION(BlueprintPure, Category = "VR Weapon | Chamber | Smoke")
	FTransform GetChamberSmokeTransform() const;

	void SetChamberState(FGameplayTag NewState);

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Chamber | Visuals")
	void UpdateVisuals();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Weapon | Chamber")
	FGameplayTag CurrentChamberState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "VR Weapon | Chamber")
	TObjectPtr<UProjectileData> LoadedProjectile;

	UPROPERTY(BlueprintReadOnly, Category = "VR Weapon | Chamber")
	TObjectPtr<UVRWeaponData> WeaponData;
};
