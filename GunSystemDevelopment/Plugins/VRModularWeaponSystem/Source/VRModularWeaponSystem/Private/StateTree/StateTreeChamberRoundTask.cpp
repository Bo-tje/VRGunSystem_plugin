#include "StateTree/StateTreeChamberRoundTask.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRChamberComponent.h"
#include "Interfaces/VRRoundProvider.h"
#include "Data/ProjectileData.h"
#include "Data/VRWeaponData.h"
#include "Core/VRWeaponBase.h"

EStateTreeRunStatus FSTTask_ChamberRound::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_ChamberRoundInstanceData& InstanceData = Context.GetInstanceData<FSTTask_ChamberRoundInstanceData>(*this);

	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	UVRChamberComponent* ChamberComponent = Weapon ? Weapon->CachedChamberComponent.Get() : nullptr;
	if (!ChamberComponent) ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>();
	if (!ChamberComponent) return EStateTreeRunStatus::Failed;
	
	UProjectileData* RoundToChamber = nullptr;

	if (Weapon)
	{
		for (UActorComponent* Component : Weapon->CachedRoundProviders)
		{
			if (Component && !Component->IsA<UVRChamberComponent>())
			{
				if (IVRRoundProvider::Execute_GetRound(Component, RoundToChamber))
				{
					break;
				}
			}
		}
	}
	else
	{
		TArray<UActorComponent*> RoundProviders;
		InstanceData.WeaponActor->GetComponents(RoundProviders);
		for (UActorComponent* Component : RoundProviders)
		{
			if (Component && Component->Implements<UVRRoundProvider>() && !Component->IsA<UVRChamberComponent>())
			{
				if (IVRRoundProvider::Execute_GetRound(Component, RoundToChamber))
				{
					break;
				}
			}
		}
	}

	if (RoundToChamber)
	{
		if (ChamberComponent->TryLoad(RoundToChamber))
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}
	else if (bInfiniteAmmo)
	{
		if (Weapon && Weapon->WeaponData && Weapon->WeaponData->DefaultProjectile)
		{
			RoundToChamber = Weapon->WeaponData->DefaultProjectile;
			if (ChamberComponent->TryLoad(RoundToChamber))
			{
				return EStateTreeRunStatus::Succeeded;
			}
		}
	}

	return EStateTreeRunStatus::Failed;
}
