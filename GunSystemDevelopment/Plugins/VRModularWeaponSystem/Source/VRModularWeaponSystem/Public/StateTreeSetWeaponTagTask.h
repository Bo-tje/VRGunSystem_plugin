#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "VRWeaponBase.h"
#include "VRChamberComponent.h"
#include "GameplayTagContainer.h"
#include "StateTreeSetWeaponTagTask.generated.h"



USTRUCT(meta=(DisplayName = "SetWeaponTag Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_SetWeaponTagInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Weapon = nullptr;
};

USTRUCT(meta=(DisplayName = "Set Weapon Tag", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_SetWeaponTag : public FStateTreeTaskBase
{
	GENERATED_BODY()

	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_SetWeaponTagInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag NewState;
};