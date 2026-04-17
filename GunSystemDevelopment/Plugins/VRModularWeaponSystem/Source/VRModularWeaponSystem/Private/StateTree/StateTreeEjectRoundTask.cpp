#include "StateTree/StateTreeEjectRoundTask.h"
#include "StateTreeExecutionContext.h"


EStateTreeRunStatus FSTTask_EjectRound::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_EjectRoundInstanceData& InstanceData = Context.GetInstanceData<FSTTask_EjectRoundInstanceData>(*this);

	if (InstanceData.WeaponActor)
	{
		if (UVRChamberComponent* Chamber = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>())
		{
			Chamber->TryEject();
			
			UE_LOG(LogTemp, Log, TEXT("StateTree: Ejecting round via Task from %s"), *InstanceData.WeaponActor->GetName());
            
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Failed;
}