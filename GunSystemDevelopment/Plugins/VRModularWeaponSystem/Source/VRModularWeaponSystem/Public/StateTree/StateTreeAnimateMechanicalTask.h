#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeAnimateMechanicalTask.generated.h"

USTRUCT(meta=(DisplayName = "Animate Mechanical Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_AnimateMechanicalInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

/**
 * Task to apply physical momentum or set state on a VRMechanicalComponent.
 * Useful for recoil animation, locking slides, or resetting parts.
 */
USTRUCT(meta=(DisplayName = "Animate Mechanical", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_AnimateMechanical : public FStateTreeTaskBase
{
	GENERATED_BODY()

	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_AnimateMechanicalInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName ComponentName;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MomentumToAdd = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bApplyMomentum = true;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bLockComponent = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bUnlockComponent = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bSetDirectValue = false;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (EditCondition = "bSetDirectValue"))
	float NewValue = 0.0f;
};
