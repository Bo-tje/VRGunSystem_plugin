#include "StateTree/StateTreeAnimateMechanicalTask.h"
#include "Components/VRMechanicalComponent.h"
#include "Core/VRWeaponBase.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTTask_AnimateMechanical::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_AnimateMechanicalInstanceData& InstanceData = Context.GetInstanceData<FSTTask_AnimateMechanicalInstanceData>(*this);

	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;

	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	if (!Weapon) return EStateTreeRunStatus::Failed;

	UVRMechanicalComponent* TargetComp = nullptr;

	// 1. Try to find the component by name in the dynamic components map
	if (UActorComponent* FoundComp = Weapon->GetDynamicComponentByName(ComponentName))
	{
		TargetComp = Cast<UVRMechanicalComponent>(FoundComp);
	}

	// 2. Fallback: Search all components if name wasn't found or was empty
	if (!TargetComp)
	{
		TArray<UVRMechanicalComponent*> MechComps;
		Weapon->GetComponents(MechComps);
		for (UVRMechanicalComponent* Comp : MechComps)
		{
			if (ComponentName.IsNone() || Comp->GetFName() == ComponentName)
			{
				TargetComp = Comp;
				break;
			}
		}
	}

	if (TargetComp)
	{
		if (bLockComponent) TargetComp->SetIsLocked(true);
		if (bUnlockComponent) TargetComp->SetIsLocked(false);

		if (bSetDirectValue)
		{
			TargetComp->SetNormalizedValue(NewValue);
		}

		if (bSetCockedState)
		{
			TargetComp->SetIsCocked(bNewCockedValue);
		}

		if (bApplyMomentum)
		{
			TargetComp->AddMomentum(MomentumToAdd);
		}

		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Failed;
}
