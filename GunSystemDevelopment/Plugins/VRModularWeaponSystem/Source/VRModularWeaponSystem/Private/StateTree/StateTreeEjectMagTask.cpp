#include "StateTree/StateTreeEjectMagTask.h"
#include "Components/VRMagwellComponent.h"
#include "StateTreeExecutionContext.h"
#include "Core/VRWeaponBase.h"

EStateTreeRunStatus FSTTask_EjectMag::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_EjectMagInstanceData& InstanceData = Context.GetInstanceData<FSTTask_EjectMagInstanceData>(*this);
	
	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	UVRMagwellComponent* MagwellComponent = Weapon ? Weapon->CachedMagwellComponent.Get() : nullptr;
	if (!MagwellComponent) MagwellComponent = InstanceData.WeaponActor->FindComponentByClass<UVRMagwellComponent>();
	if (!MagwellComponent) return EStateTreeRunStatus::Failed;

	MagwellComponent->EjectMagazine();
	
	return EStateTreeRunStatus::Succeeded;
}
