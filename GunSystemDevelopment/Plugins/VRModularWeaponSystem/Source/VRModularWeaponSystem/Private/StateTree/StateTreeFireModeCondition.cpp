#include "StateTree/StateTreeFireModeCondition.h"
#include "StateTreeExecutionContext.h"

bool FSTCondition_FireMode::TestCondition(FStateTreeExecutionContext& Context) const
{
	FSTCondition_FireModeInstanceData& InstanceData = Context.GetInstanceData<FSTCondition_FireModeInstanceData>(*this);

	if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor))
	{
		FVRFireMode CurrentMode = Weapon->GetCurrentFireMode();

		// This condition can be expanded to check BurstCount, etc.
		// For now, it simply ensures the fire mode is valid and returns true,
		// but allows the StateTree to branch based on mode properties.
		return CurrentMode.ModeName != NAME_None;
	}

	return false;
}
