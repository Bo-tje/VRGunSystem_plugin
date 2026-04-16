#include "StateTreeWeaponEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "VRChamberComponent.h"
#include "VRWeaponData.h"

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
		
		if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor))
		{
			// Calculate the delay in seconds (60 seconds / Rounds Per Minute)
			if (Weapon->WeaponData && Weapon->WeaponData->FireRate > 0)
			{
				InstanceData.TimeBetweenShots = 60.0f / Weapon->WeaponData->FireRate;
			}
		}
	}	
}

