#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRMechanicalComponent.h"
#include "StateTreeMechanicalCondition.generated.h"

USTRUCT()
struct VRMODULARWEAPONSYSTEM_API FSTCondition_MechanicalInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName="Check Mechanical State", Category="Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTCondition_MechanicalState : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTCondition_MechanicalInstanceData::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName ComponentName;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bCheckIsHeld = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIsHeldValue = true;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bCheckIsLocked = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIsLockedValue = true;
};
