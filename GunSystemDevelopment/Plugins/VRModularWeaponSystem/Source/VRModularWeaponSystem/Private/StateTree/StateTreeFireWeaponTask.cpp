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
	
	UVRFireComponent* FireComponent = InstanceData.WeaponActor->FindComponentByClass<UVRFireComponent>();
	if (!FireComponent) return EStateTreeRunStatus::Failed;
	
	UProjectileData* RoundToFire = nullptr;
	
	if (UVRChamberComponent* ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>())
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
	
if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor))
	{
		for (UActorComponent* Component : Weapon->CachedRoundProviders)
		{
			if (Component && !Component->IsA<UVRChamberComponent>())
			{
				if (IVRRoundProvider::Execute_GetRound(Component, RoundToFire))
				{
					FireComponent->HandleFiring(RoundToFire);
					return EStateTreeRunStatus::Succeeded;
				}

				FireComponent->HandleDryFire();

				return EStateTreeRunStatus::Failed;
			}
		}
	}
	
	FireComponent->HandleFiring();
	return EStateTreeRunStatus::Succeeded;
}
