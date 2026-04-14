#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VRWeaponBase.h"
#include "StateTreeChamberRoundTask.generated.h"

USTRUCT(meta= (DisplayName= "Chamber Round Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_ChamberRoundInstanceData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta = (DisplayName = "Chamber Round", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_ChamberRound : public FStateTreeTaskBase
{
	GENERATED_BODY()
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_ChamberRoundInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInfiniteAmmo = false;
};
