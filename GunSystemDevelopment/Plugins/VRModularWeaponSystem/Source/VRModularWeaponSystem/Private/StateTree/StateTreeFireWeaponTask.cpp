#include "StateTree/StateTreeFireWeaponTask.h"
#include "Components/VRMechanicalComponent.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRFireComponent.h"
#include "Interfaces/VRRoundProvider.h"
#include "Core/VRWeaponBase.h"


EStateTreeRunStatus FSTTask_FireWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_FireWeaponInstanceData& InstanceData = Context.GetInstanceData<FSTTask_FireWeaponInstanceData>(*this);
	
	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	UVRFireComponent* FireComponent = Weapon ? Weapon->CachedFireComponent.Get() : nullptr;
	if (!FireComponent) FireComponent = InstanceData.WeaponActor->FindComponentByClass<UVRFireComponent>();
	if (!FireComponent) return EStateTreeRunStatus::Failed;
	
	UProjectileData* RoundToFire = nullptr;
	
	UVRChamberComponent* ChamberComponent = Weapon ? Weapon->CachedChamberComponent.Get() : nullptr;
	if (!ChamberComponent) ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>();
	if (ChamberComponent)
	{
		if (IVRRoundProvider::Execute_GetRound(ChamberComponent, RoundToFire))
		{
			FireComponent->HandleFiring(RoundToFire);
			return EStateTreeRunStatus::Succeeded;
		}

		if (bOnlyFireFromChamber)
		{
			FireComponent->HandleDryFire();
			return EStateTreeRunStatus::Failed;
		}
	}
	
	if (Weapon)
	{
		bool bHasRoundProviders = false;
		for (UActorComponent* Component : Weapon->CachedRoundProviders)
		{
			if (Component && !Component->IsA<UVRChamberComponent>())
			{
				bHasRoundProviders = true;
				if (IVRRoundProvider::Execute_GetRound(Component, RoundToFire))
				{
					FireComponent->HandleFiring(RoundToFire);
					return EStateTreeRunStatus::Succeeded;
				}
			}
		}

		if (bHasRoundProviders)
		{
			FireComponent->HandleDryFire();
			return EStateTreeRunStatus::Failed;
		}
	}
	
	FireComponent->HandleFiring();
	return EStateTreeRunStatus::Succeeded;
}
