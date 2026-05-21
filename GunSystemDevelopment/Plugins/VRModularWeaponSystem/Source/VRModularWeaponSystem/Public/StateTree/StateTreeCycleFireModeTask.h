#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Core/VRWeaponBase.h"
#include "StateTreeCycleFireModeTask.generated.h"

USTRUCT(meta=(DisplayName = "Cycle Fire Mode Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_CycleFireModeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName = "Cycle Fire Mode", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_CycleFireMode : public FStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_CycleFireModeInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
protected:
	/** If true, cycles backward through the fire modes. */
	UPROPERTY(EditAnywhere, Category = "Parameter") 
	bool bCycleBackward = false;

	/** If target index is >= 0, sets the fire mode to that specific index instead of cycling. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	int32 TargetIndex = -1;
};
