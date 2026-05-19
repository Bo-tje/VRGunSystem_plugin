#include "StateTree/StateTreeMechanicalCondition.h"
#include "StateTreeExecutionContext.h"

bool FSTCondition_MechanicalState::TestCondition(FStateTreeExecutionContext& Context) const
{
	FSTCondition_MechanicalInstanceData& InstanceData = Context.GetInstanceData<FSTCondition_MechanicalInstanceData>(*this);

	if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor))
	{
		UVRMechanicalComponent* TargetComp = nullptr;

		if (!ComponentName.IsNone())
		{
			TargetComp = Cast<UVRMechanicalComponent>(Weapon->GetDynamicComponentByName(ComponentName));
		}
		
		// Fallback: If no name provided, grab the first mechanical component that isn't the trigger
		if (!TargetComp)
		{
			for (UActorComponent* Comp : Weapon->CachedWeaponComponents)
			{
				if (UVRMechanicalComponent* MechComp = Cast<UVRMechanicalComponent>(Comp))
				{
					// Simple heuristic: triggers usually don't have large max ranges or are explicitly named Trigger
					if (MechComp->GetFName() != TEXT("Trigger"))
					{
						TargetComp = MechComp;
						break;
					}
				}
			}
		}

		if (TargetComp)
		{
			if (bCheckIsHeld)
			{
				if (TargetComp->bIsBeingHeld != bIsHeldValue) return false;
			}

			if (bCheckIsLocked)
			{
				if (TargetComp->bIsLocked != bIsLockedValue) return false;
			}

			return true;
		}
	}

	return false;
}
