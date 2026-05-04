#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Interfaces/VRWeaponInterface.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Data/VRWeaponData.h"
#include "VRFireComponent.generated.h"

class UVRWeaponComponentSettings;
class UProjectileData;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponDryFired);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRFireComponent : public USceneComponent, public IVRWeaponComponentInterface, public IVRWeaponInterface
{
	GENERATED_BODY()

public:	
	UVRFireComponent();

	virtual void OnRegister() override;

	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override;
	virtual void InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Config")
	FName MuzzleSocketName = TEXT("Muzzle");
	
	UPROPERTY(EditAnywhere, Category = "VR Weapon | Config")
	float FireHapticScale = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "VR Weapon | Config")
	float DryFireHapticScale = .5f;

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Logic")
	FTransform GetMuzzleTransform() const;


	UPROPERTY()
	TObjectPtr<UVRWeaponData> WeaponData;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Events")
	FOnWeaponFired OnFired;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Events")
	FOnWeaponDryFired OnDryFired;
	


	// IVRWeaponInterface
	virtual void PullTrigger_Implementation() override;
	virtual void ReleaseTrigger_Implementation() override;
	virtual void PrimaryAction_Implementation() override {}
	virtual void ReleasePrimaryAction_Implementation() override {}
	virtual void SecondaryAction_Implementation() override {}
	virtual void ReleaseSecondaryAction_Implementation() override {}
	virtual bool IsTriggerPulled_Implementation() const override { return false; }

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Actions")
	void HandleFiring(UProjectileData* ProjectileData = nullptr);

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Actions")
	void HandleDryFire();
	
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Actions")
	void PlayHaptics(UHapticFeedbackEffect_Base* HapticEffect, float HapticScale);

	void PerformHitscan(const UProjectileData* Data, const FVector& StartLocation, const FRotator& StartRotation) const;
	
private:
	UPROPERTY()
	TObjectPtr<UAudioComponent> FireAudioComponent;
	UPROPERTY()
	TObjectPtr<UAudioComponent> DryFireAudioComponent;
	
	

};
