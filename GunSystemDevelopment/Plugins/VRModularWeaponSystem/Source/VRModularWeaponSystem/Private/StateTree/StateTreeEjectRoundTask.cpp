#include "StateTree/StateTreeEjectRoundTask.h"
#include "StateTreeExecutionContext.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRChamberComponent.h"


EStateTreeRunStatus FSTTask_EjectRound::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_EjectRoundInstanceData& InstanceData = Context.GetInstanceData<FSTTask_EjectRoundInstanceData>(*this);

	if (InstanceData.WeaponActor)
	{
		AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
		UVRChamberComponent* Chamber = Weapon ? Weapon->CachedChamberComponent.Get() : nullptr;
		if (!Chamber) Chamber = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>();
		if (Chamber)
		{
			Chamber->TryEject();
			
			UE_LOG(LogTemp, Log, TEXT("StateTree: Ejecting round via Task from %s"), *InstanceData.WeaponActor->GetName());
            
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Failed;
}