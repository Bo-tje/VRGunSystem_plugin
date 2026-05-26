#pragma once

#include "CoreMinimal.h"
#include "DemoTargets/VRTargetBase.h"
#include "VRTargetWithHealth.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetDestroyedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetHealthChangedSignature, float, NewHealth, float, MaxHealth);

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVRTargetWithHealth : public AVRTargetBase
{
	GENERATED_BODY()

public:
	AVRTargetWithHealth();

protected:
	virtual void BeginPlay() override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void ResetTarget() override;

	/** Destroys the target, triggering effects, physics, or deactivation */
	UFUNCTION(BlueprintCallable, Category = "VR Target")
	virtual void DestroyTarget();

	UFUNCTION(BlueprintPure, Category = "VR Target")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "VR Target")
	float GetMaxHealth() const { return MaxHealth; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health")
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target | Health")
	float CurrentHealth;

	/** If true, target only performs knockdown rotation when health reaches 0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health")
	bool bKnockdownOnlyOnDestroy;

	/** If true, target will simulate physics and tumble to the ground on death */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health | Destruction")
	bool bSimulatePhysicsOnDestroy;

	/** If true, target mesh is hidden on death (e.g. for fracturing or explosion effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health | Destruction")
	bool bHideMeshOnDestroy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health | Destruction")
	TObjectPtr<USoundBase> DestroySound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health | Destruction")
	TObjectPtr<UNiagaraSystem> DestroyEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Health | Reset")
	bool bShouldReset;

	UPROPERTY(BlueprintAssignable, Category = "VR Target | Events")
	FOnTargetDestroyedSignature OnTargetDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "VR Target | Events")
	FOnTargetHealthChangedSignature OnTargetHealthChanged;

private:
	bool bIsDestroyed;
	FTransform InitialMeshTransform;
};
