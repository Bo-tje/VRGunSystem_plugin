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
	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Config")
	FName MuzzleSocketName = TEXT("Muzzle");

	/** Helper to get the best available fire transform */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Logic")
	FTransform GetMuzzleTransform() const;
	
	
	UPROPERTY()
	UVRWeaponData* WeaponData;
	
	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Events")
	FOnWeaponFired OnFired;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Events")
	FOnWeaponDryFired OnDryFired;
	
	
	virtual void PullTrigger_Implementation();
	virtual void ReleaseTrigger_Implementation();

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Actions")
	void HandleFiring(UProjectileData* ProjectileData) const;


private:
	void PerformHitscan(const UProjectileData* Data, const FVector& StartLocation, const FRotator& StartRotation) const;
	static void SpawnProjectile(UProjectileData* Data);
};
