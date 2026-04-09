#include "StateTreeEjectRoundTask.h"
#include "StateTreeExecutionContext.h"


EStateTreeRunStatus FSTTask_EjectRound::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 1. Get our Instance Data (the weapon reference)
	FSTTask_EjectRoundInstanceData& InstanceData = Context.GetInstanceData<FSTTask_EjectRoundInstanceData>(*this);

	if (InstanceData.Weapon)
	{
		// 2. Find the Chamber Component on the weapon
		if (UVRChamberComponent* Chamber = InstanceData.Weapon->FindComponentByClass<UVRChamberComponent>())
		{
			// 3. Perform the physical action
			Chamber->TryEject();
            
			// Log for the developer to see the tree working
			UE_LOG(LogTemp, Log, TEXT("StateTree: Ejecting round via Task from %s"), *InstanceData.Weapon->GetName());
            
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Failed;
}