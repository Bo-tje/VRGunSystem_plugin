#include "VRNativeTags.h"


namespace VRNativeTags
{
#pragma region Interaction Tags
	UE_DEFINE_GAMEPLAY_TAG(Trigger, "VRModularWeaponSystem.Interaction.Trigger");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInput, "VRModularWeaponSystem.Interaction.PrimaryInput");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInput, "VRModularWeaponSystem.Interaction.SecondaryInput");
	

#pragma endregion
	
#pragma region Weapon states
	
	UE_DEFINE_GAMEPLAY_TAG(Idle, "VRModularWeaponSystem.WeaponState.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Held, "VRModularWeaponSystem.WeaponState.Held");
	UE_DEFINE_GAMEPLAY_TAG(Firing, "VRModularWeaponSystem.WeaponState.Firing");
	UE_DEFINE_GAMEPLAY_TAG(Reloading, "VRModularWeaponSystem.WeaponState.Reloading");
	UE_DEFINE_GAMEPLAY_TAG(LockedBack, "VRModularWeaponSystem.WeaponState.LockedBack");
	UE_DEFINE_GAMEPLAY_TAG(Jammed, "VRModularWeaponSystem.WeaponState.Jammed");
	UE_DEFINE_GAMEPLAY_TAG(Empty, "VRModularWeaponSystem.WeaponState.Empty");
	
#pragma endregion 
	
#pragma region Chamber states	

	UE_DEFINE_GAMEPLAY_TAG(Chamber_Empty, "VRModularWeaponSystem.ChamberState.Empty");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_RoundReady, "VRModularWeaponSystem.ChamberState.RoundReady");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_SpentCasing, "VRModularWeaponSystem.ChamberState.SpentCasing");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_Jammed, "VRModularWeaponSystem.ChamberState.Jammed");
	
#pragma endregion
}
