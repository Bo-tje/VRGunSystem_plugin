#include "StateTree/StateTreeFireModeCondition.h"
#include "StateTreeExecutionContext.h"

bool FSTCondition_FireMode::TestCondition(FStateTreeExecutionContext& Context) const
{
	FSTCondition_FireModeInstanceData& InstanceData = Context.GetInstanceData<FSTCondition_FireModeInstanceData>(*this);

	if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor))
	{
		FVRFireMode CurrentMode = Weapon->GetCurrentFireMode();

		if (bCheckIsAutomatic)
		{
			if (CurrentMode.bIsAutomatic != bIsAutomaticValue) return false;
		}

		if (bCheckModeName)
		{
			if (CurrentMode.ModeName != ModeName) return false;
		}

		return true;
	}

	return false;
}
