#include "StateTreeFireWeaponTask.h"
#include "StateTreeExecutionContext.h"
#include "VRFireComponent.h"
#include "VRRoundProvider.h"


EStateTreeRunStatus FSTTask_FireWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_FireWeaponInstanceData& InstanceData = Context.GetInstanceData<FSTTask_FireWeaponInstanceData>(*this);

	if (InstanceData.Weapon)
	{
		for (UActorComponent* Component : InstanceData.Weapon->GetComponents())
		{
			// Use Implements and Execute instead of Cast for Interfaces
			if (Component && Component->Implements<UVRRoundProvider>())
			{
				UProjectileData* RoundToFire = nullptr;
				if (IVRRoundProvider::Execute_GetRound(Component, RoundToFire))
				{
					if (UVRFireComponent* FireComponent = InstanceData.Weapon->FindComponentByClass<UVRFireComponent>())
					{
						FireComponent->HandleFiring(RoundToFire);
						return EStateTreeRunStatus::Succeeded;
					}
				}
				break; 
			}
		}
	}

	return EStateTreeRunStatus::Failed;
}