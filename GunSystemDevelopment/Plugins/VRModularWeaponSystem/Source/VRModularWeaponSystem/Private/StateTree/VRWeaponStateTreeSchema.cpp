#include "StateTree/VRWeaponStateTreeSchema.h"
#include "StateTreeTaskBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeConditionBase.h"
#include "Data/VRWeaponData.h"

UVRWeaponStateTreeSchema::UVRWeaponStateTreeSchema()
{
	WeaponDataDescription.Name = TEXT("WeaponData");
	WeaponDataDescription.Struct = UVRWeaponData::StaticClass();
	WeaponDataDescription.Requirement = EStateTreeExternalDataRequirement::Required;
}

bool UVRWeaponStateTreeSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	return InScriptStruct->IsChildOf(FStateTreeTaskBase::StaticStruct())
	|| InScriptStruct->IsChildOf(FStateTreeEvaluatorBase::StaticStruct())
	|| InScriptStruct->IsChildOf(FStateTreeConditionBase::StaticStruct());
}
