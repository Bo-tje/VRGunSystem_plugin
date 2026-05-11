#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "Core/VRWeaponBase.h"
#include "StateTreeFireModeCondition.generated.h"

USTRUCT()
struct VRMODULARWEAPONSYSTEM_API FSTCondition_FireModeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName="Check Fire Mode", Category="Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTCondition_FireMode : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTCondition_FireModeInstanceData::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bCheckIsAutomatic = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bIsAutomaticValue = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bCheckModeName = false;

	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (EditCondition = "bCheckModeName"))
	FName ModeName;
};

