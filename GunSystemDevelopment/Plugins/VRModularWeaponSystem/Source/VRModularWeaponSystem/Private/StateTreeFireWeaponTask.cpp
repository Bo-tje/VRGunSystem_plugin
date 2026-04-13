#include "StateTreeFireWeaponTask.h"
#include "StateTreeExecutionContext.h"
#include "VRFireComponent.h"
#include "VRRoundProvider.h"


EStateTreeRunStatus FSTTask_FireWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_FireWeaponInstanceData& InstanceData = Context.GetInstanceData<FSTTask_FireWeaponInstanceData>(*this);

	if (!InstanceData.Weapon) return EStateTreeRunStatus::Failed;
	
	UVRFireComponent* FireComponent = InstanceData.Weapon->FindComponentByClass<UVRFireComponent>();
	if (!FireComponent) return EStateTreeRunStatus::Failed;
	
	UProjectileData* RoundToFire = nullptr;
	
	if (UVRChamberComponent* ChamberComponent = InstanceData.Weapon->FindComponentByClass<UVRChamberComponent>())
	{
		if (IVRRoundProvider::Execute_GetRound(ChamberComponent, RoundToFire))
		{
			FireComponent->HandleFiring(RoundToFire);
			return EStateTreeRunStatus::Succeeded;
		}

		if (bOnlyFireFromChamber)
		{
			FireComponent->OnDryFired.Broadcast();
			return EStateTreeRunStatus::Failed;
		}
	}
	
	TArray<UActorComponent*> RoundProviders;
	InstanceData.Weapon->GetComponents(RoundProviders);
	
	for (UActorComponent* Component : RoundProviders)
	{
		if (Component && Component->Implements<UVRRoundProvider>() && !Component->IsA<UVRChamberComponent>() && !Component->IsA<UVRFireComponent>())
		{
			if (IVRRoundProvider::Execute_GetRound(Component, RoundToFire))
			{
				FireComponent->HandleFiring(RoundToFire);
				return EStateTreeRunStatus::Succeeded;
			}
		}
	}
	FireComponent->OnDryFired.Broadcast();
	return EStateTreeRunStatus::Failed;
}