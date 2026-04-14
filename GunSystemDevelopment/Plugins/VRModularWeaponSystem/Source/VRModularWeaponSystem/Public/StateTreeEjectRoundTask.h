#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VRWeaponBase.h"
#include "VRChamberComponent.h"
#include "StateTreeEjectRoundTask.generated.h"



USTRUCT(meta=(DisplayName= "Eject Round Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_EjectRoundInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName= "Eject Round", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_EjectRound : public FStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_EjectRoundInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};