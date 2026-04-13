#include "StateTreeWeaponEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "VRChamberComponent.h"

void FSTEval_Weapon::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FStateTreeEvaluatorBase::Tick(Context, DeltaTime);
	
	FSTEval_WeaponInstanceData& InstanceData = Context.GetInstanceData<FSTEval_WeaponInstanceData>(*this);

	if (InstanceData.WeaponActor)
	{
		if (InstanceData.WeaponActor->Implements<UVRWeaponInterface>())
		{
			InstanceData.bIsTriggerPulled = IVRWeaponInterface::Execute_IsTriggerPulled(InstanceData.WeaponActor);
		}

		if (const UVRChamberComponent* ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>())
		{
			InstanceData.ChamberStateTag = ChamberComponent->GetChamberState();
			InstanceData.bHasRoundReady = ChamberComponent->IsRoundReady();
		}
		
	}	
}
