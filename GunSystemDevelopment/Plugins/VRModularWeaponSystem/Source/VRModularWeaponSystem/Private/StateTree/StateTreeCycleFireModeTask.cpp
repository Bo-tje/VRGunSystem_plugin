#include "StateTree/StateTreeCycleFireModeTask.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTTask_CycleFireMode::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_CycleFireModeInstanceData& InstanceData = Context.GetInstanceData<FSTTask_CycleFireModeInstanceData>(*this);
	
	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	if (!Weapon) return EStateTreeRunStatus::Failed;

	if (TargetIndex >= 0)
	{
		Weapon->SetFireModeIndex(TargetIndex);
	}
	else
	{
		Weapon->CycleFireMode(bCycleBackward);
	}

	return EStateTreeRunStatus::Succeeded;
}
