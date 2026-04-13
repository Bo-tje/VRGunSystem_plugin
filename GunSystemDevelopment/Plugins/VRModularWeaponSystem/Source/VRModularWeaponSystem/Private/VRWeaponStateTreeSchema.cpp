#include "VRWeaponStateTreeSchema.h"
#include "VRWeaponBase.h"
#include "VRWeaponData.h"

UVRWeaponStateTreeSchema::UVRWeaponStateTreeSchema()
{
	WeaponActorDescription.Name = TEXT("WeaponActor");
	WeaponActorDescription.Struct = AVRWeaponBase::StaticClass();
	WeaponActorDescription.Requirement = EStateTreeExternalDataRequirement::Required;

	WeaponDataDescription.Name = TEXT("WeaponData");
	WeaponDataDescription.Struct = UVRWeaponData::StaticClass();
	WeaponDataDescription.Requirement = EStateTreeExternalDataRequirement::Optional;
}
