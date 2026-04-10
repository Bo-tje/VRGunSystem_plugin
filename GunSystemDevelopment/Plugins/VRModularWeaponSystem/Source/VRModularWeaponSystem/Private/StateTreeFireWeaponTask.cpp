#include "StateTreeFireWeaponTask.h"
#include "StateTreeExecutionContext.h"
#include "VRFireComponent.h"
#include "VRRoundProvider.h"


EStateTreeRunStatus FSTTask_FireWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 1. Get our Instance Data (the weapon reference)
	FSTTask_FireWeaponInstanceData& InstanceData = Context.GetInstanceData<FSTTask_FireWeaponInstanceData>(*this);

	if (InstanceData.Weapon)
	{
		// Look for any component that implements your provider interface
		for (UActorComponent* Comp : InstanceData.Weapon->GetComponents())
		{
			if (IVRRoundProvider* RoundProvider = Cast<IVRRoundProvider>(Comp))
			{
				UProjectileData* RoundToFire = nullptr;
				if (RoundProvider->Execute_GetRound(Comp, RoundToFire))
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