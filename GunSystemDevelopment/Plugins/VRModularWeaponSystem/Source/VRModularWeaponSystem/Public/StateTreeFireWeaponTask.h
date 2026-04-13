#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VRWeaponBase.h"
#include "VRChamberComponent.h"
#include "StateTreeFireWeaponTask.generated.h"



USTRUCT()
struct VRMODULARWEAPONSYSTEM_API FSTTask_FireWeaponInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AVRWeaponBase> Weapon = nullptr;
};

USTRUCT()
struct VRMODULARWEAPONSYSTEM_API FSTTask_FireWeapon : public FStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_FireWeaponInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Parameter") 
	bool bOnlyFireFromChamber = false;
};