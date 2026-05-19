#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeEjectMagTask.generated.h"

USTRUCT(meta=(DisplayName = "Eject Mag Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_EjectMagInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName = "Eject Magazine", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_EjectMag : public FStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_EjectMagInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
