#include "StateTreeFireWeaponTask.h"
#include "StateTreeExecutionContext.h"`
#include "VRFireComponent.h"
#include "VRRoundProvider.h"


EStateTreeRunStatus FSTTask_FireWeapon::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 1. Get our Instance Data (the weapon reference)
	FSTTask_FireWeaponInstanceData& InstanceData = Context.GetInstanceData<FSTTask_FireWeaponInstanceData>(*this);

	if (InstanceData.Weapon)
	{
		// Look for any component that implements your provider interface
		UVRRoundProvider* ProviderComp = InstanceData.Weapon->FindComponentByClass<UVRRoundProvider>();
		if (IVRRoundProvider* RoundProvider = Cast<IVRRoundProvider>(ProviderComp))
		{
			UProjectileData* RoundToFire = nullptr;
			if (RoundProvider->Execute_GetRound(ProviderComp, RoundToFire))
				{
					if (UVRFireComponent* FireComponent = Cast<UVRFireComponent>(InstanceData.Weapon->FindComponentByClass<UVRFireComponent>()))
					{
						FireComponent->HandleFiring(RoundToFire);
						return EStateTreeRunStatus::Succeeded;
					}
			}
		}
	}

	return EStateTreeRunStatus::Failed;
}