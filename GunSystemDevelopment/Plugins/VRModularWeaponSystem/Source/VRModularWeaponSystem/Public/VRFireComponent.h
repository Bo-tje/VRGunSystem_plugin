#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "VRWeaponComponentInterface.h"
#include "VRFireComponent.generated.h"

class UVRWeaponData;
class UProjectileData;
class UStaticMeshComponent;

// Multicast delegates for other modular components to listen to
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponDryFired);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRFireComponent : public USceneComponent, public IVRWeaponComponentInterface
{
	GENERATED_BODY()

public:	
	UVRFireComponent();

protected:
	virtual void BeginPlay() override;
	virtual void OnRegister() override;

/*
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
*/
	
public:	
	
	// --- IVRWeaponComponentInterface ---
	virtual void InitializeComponent(UVRWeaponData* InData);

	// The point from which projectiles/hitscans originate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Visuals")
	USceneComponent* MuzzleLocation;
	
	
	UPROPERTY()
	UVRWeaponData* WeaponData;
	
	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Events")
	FOnWeaponFired OnFired;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Events")
	FOnWeaponDryFired OnDryFired;
	
	
	virtual void PullTrigger_Implementation();
	virtual void ReleaseTrigger_Implementation();

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Actions")
	void HandleFiring();


private:
	void PerformHitscan(const UVRWeaponData* ProjectileInfo ) const;
	void SpawnProjectile(UProjectileData* ProjectileInfo);
};
