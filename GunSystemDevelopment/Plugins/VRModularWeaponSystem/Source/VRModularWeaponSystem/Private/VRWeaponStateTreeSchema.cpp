#include "VRWeaponStateTreeSchema.h"
#include "VRWeaponData.h"

UVRWeaponStateTreeSchema::UVRWeaponStateTreeSchema()
{
	WeaponDataDescription.Name = TEXT("WeaponData");
	WeaponDataDescription.Struct = UVRWeaponData::StaticClass();
	WeaponDataDescription.Requirement = EStateTreeExternalDataRequirement::Required;
}
