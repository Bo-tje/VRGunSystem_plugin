#include "VRWeaponStateTreeComponent.h"
#include "StateTreeExecutionContext.h"
#include "VRWeaponBase.h"
#include "VRWeaponData.h"
#include "VRWeaponStateTreeSchema.h"

TSubclassOf<UStateTreeSchema> UVRWeaponStateTreeComponent::GetSchema() const
{
	return UVRWeaponStateTreeSchema::StaticClass();
}

bool UVRWeaponStateTreeComponent::SetContextRequirements(FStateTreeExecutionContext& Context, bool bLogErrors)
{
	// 1. Let the base component handle standard Actor requirements
	if (!Super::SetContextRequirements(Context, bLogErrors))
	{
		return false;
	}

	// 2. Bind the WeaponData from the owning actor
	if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(GetOwner()))
	{
		if (Weapon->WeaponData)
		{
			// This method is correctly exported in the StateTree module
			Context.SetContextDataByName(TEXT("WeaponData"), FStateTreeDataView(Weapon->WeaponData));
		}
	}

	return true;
}
