#include "StateTree/StateTreeWeaponEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRChamberComponent.h"
#include "Data/VRWeaponData.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRWeaponStateTreeComponent.h"

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

		if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor))
		{
			const UVRChamberComponent* ChamberComponent = Weapon->CachedChamberComponent.Get();
			if (!ChamberComponent)
			{
				ChamberComponent = Weapon->FindComponentByClass<UVRChamberComponent>();
				// Self-heal: re-cache so subsequent tasks in this frame don't need to fallback
				if (ChamberComponent)
				{
					Weapon->CachedChamberComponent = const_cast<UVRChamberComponent*>(ChamberComponent);
				}
			}
			if (ChamberComponent)
			{
				InstanceData.ChamberStateTag = ChamberComponent->GetChamberState();
				InstanceData.bHasRoundReady = ChamberComponent->IsRoundReady();
			}

			// Calculate the delay in seconds (60 seconds / Rounds Per Minute)
			float FireRate = Weapon->GetCalculatedStats().FireRate;
			if (FireRate > 0.0f)
			{
				InstanceData.TimeBetweenShots = 60.0f / FireRate;
			}
		}
		else
		{
			if (const UVRChamberComponent* ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>())
			{
				InstanceData.ChamberStateTag = ChamberComponent->GetChamberState();
				InstanceData.bHasRoundReady = ChamberComponent->IsRoundReady();
			}
		}
	}	
}

