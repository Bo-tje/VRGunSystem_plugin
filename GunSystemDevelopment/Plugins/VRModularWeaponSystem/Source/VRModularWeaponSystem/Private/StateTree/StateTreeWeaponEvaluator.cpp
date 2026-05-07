#include "StateTree/StateTreeWeaponEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRChamberComponent.h"
#include "Data/VRWeaponData.h"
#include "Core/VRWeaponBase.h"

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
			float FireRate = Weapon->GetCalculatedStats().FireRate;
			if (FireRate > 0.0f)
			{
				InstanceData.TimeBetweenShots = 60.0f / FireRate;
			}
		}
	}	
}

