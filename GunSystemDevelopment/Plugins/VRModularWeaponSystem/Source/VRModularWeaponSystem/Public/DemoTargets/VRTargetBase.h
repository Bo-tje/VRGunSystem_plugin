#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "VRTargetBase.generated.h"

class UStaticMeshComponent;
class USoundBase;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnTargetHitSignature, float, Damage, FVector, HitLocation, FVector, HitNormal, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargetStateChangedSignature);

UENUM(BlueprintType)
enum class EVRTargetState : uint8
{
	Active,
	KnockedDown,
	Resetting
};

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVRTargetBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AVRTargetBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	/** Reset the target back to its active standing state */
	UFUNCTION(BlueprintCallable, Category = "VR Target")
	virtual void ResetTarget();

	/** Instantly knocks down the target */
	UFUNCTION(BlueprintCallable, Category = "VR Target")
	virtual void KnockdownTarget();

	/** Gets the current target state */
	UFUNCTION(BlueprintPure, Category = "VR Target")
	EVRTargetState GetTargetState() const { return CurrentState; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TargetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Audio")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | VFX")
	TObjectPtr<UNiagaraSystem> HitEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Configuration")
	bool bKnockdownOnHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Configuration", meta = (EditCondition = "bKnockdownOnHit"))
	float KnockdownAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Configuration", meta = (EditCondition = "bKnockdownOnHit"))
	float KnockdownSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Configuration", meta = (EditCondition = "bKnockdownOnHit"))
	float ResetDelay;

	UPROPERTY(BlueprintAssignable, Category = "VR Target | Events")
	FOnTargetHitSignature OnTargetHit;

	UPROPERTY(BlueprintAssignable, Category = "VR Target | Events")
	FOnTargetStateChangedSignature OnTargetKnockedDown;

	UPROPERTY(BlueprintAssignable, Category = "VR Target | Events")
	FOnTargetStateChangedSignature OnTargetReset;

	/** State management */
	UPROPERTY(BlueprintReadOnly, Category = "VR Target | State")
	EVRTargetState CurrentState;

	FRotator TargetMeshRotation;
	FRotator DefaultMeshRotation;
	FTimerHandle ResetTimerHandle;
};
