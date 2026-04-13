#include "StateTreeSetWeaponTagTask.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "VRWeaponInterface.h"

EStateTreeRunStatus FSTTask_SetWeaponTag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_SetWeaponTagInstanceData& InstanceData = Context.GetInstanceData<FSTTask_SetWeaponTagInstanceData>(*this);

	if (InstanceData.Weapon && InstanceData.Weapon->Implements<UVRWeaponInterface>())
	{ 
		IVRWeaponInterface::Execute_SetWeaponState(InstanceData.Weapon, NewState);
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}