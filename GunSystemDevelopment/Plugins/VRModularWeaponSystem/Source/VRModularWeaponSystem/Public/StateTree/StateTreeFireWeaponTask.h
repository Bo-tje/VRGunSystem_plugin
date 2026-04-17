#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRChamberComponent.h"
#include "StateTreeFireWeaponTask.generated.h"



USTRUCT(meta=(DisplayName = "Fire Weapon Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_FireWeaponInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName = "Fire Weapon", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_FireWeapon : public FStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_FireWeaponInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Parameter") 
	bool bOnlyFireFromChamber = false;
};