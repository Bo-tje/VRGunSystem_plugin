#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VRWeaponBase.h"
#include "VRChamberComponent.h"
#include "StateTreeEjectRoundTask.generated.h"



// This is the "Instance Data" - it holds the actual references during runtime
USTRUCT()
struct VRMODULARWEAPONSYSTEM_API FSTTask_EjectRoundInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AVRWeaponBase> Weapon = nullptr;
};

USTRUCT()
struct VRMODULARWEAPONSYSTEM_API FSTTask_EjectRound : public FStateTreeTaskBase
{
	GENERATED_BODY()

	// This defines what the task needs from the weapon to function
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_EjectRoundInstanceData::StaticStruct(); }

	// This runs the moment the State Tree enters the node
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};