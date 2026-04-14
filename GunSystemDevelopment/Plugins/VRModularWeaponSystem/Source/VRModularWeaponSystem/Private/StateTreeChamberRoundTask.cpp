#include "StateTreeChamberRoundTask.h"
#include "StateTreeExecutionContext.h"
#include "VRChamberComponent.h"
#include "VRRoundProvider.h"
#include "ProjectileData.h"

EStateTreeRunStatus FSTTask_ChamberRound::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_ChamberRoundInstanceData& InstanceData = Context.GetInstanceData<FSTTask_ChamberRoundInstanceData>(*this);

	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	UVRChamberComponent* ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>();
	if (!ChamberComponent) return EStateTreeRunStatus::Failed;
	
	UProjectileData* RoundToChamber = nullptr;
	TArray<UActorComponent*> RoundProviders;
	InstanceData.WeaponActor->GetComponents(RoundProviders);
	
	for (UActorComponent* Component : RoundProviders)
	{
		if (Component && Component->Implements<UVRRoundProvider>() && Component != ChamberComponent)
		{
			if (IVRRoundProvider::Execute_GetRound(Component, RoundToChamber))
			{
				break;
			}
		}
	}

	if (RoundToChamber)
	{
		if (ChamberComponent->TryLoad(RoundToChamber))
		{
			UE_LOG(LogTemp, Warning, TEXT("Loaded %s into chamber"), *RoundToChamber->GetName());
			return EStateTreeRunStatus::Succeeded;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Failed to load round into chamber"));
	return EStateTreeRunStatus::Failed;
}