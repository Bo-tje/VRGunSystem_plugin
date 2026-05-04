#include "Core/VRNativeTags.h"


namespace VRNativeTags
{
#pragma region Interaction Tags
	UE_DEFINE_GAMEPLAY_TAG(Trigger, "VRModularWeaponSystem.Interaction.Trigger");
	UE_DEFINE_GAMEPLAY_TAG(TriggerReleased, "VRModularWeaponSystem.Interaction.TriggerReleased");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInput, "VRModularWeaponSystem.Interaction.PrimaryInput");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInputReleased, "VRModularWeaponSystem.Interaction.PrimaryInputReleased");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInput, "VRModularWeaponSystem.Interaction.SecondaryInput");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInputReleased, "VRModularWeaponSystem.Interaction.SecondaryInputReleased");
	

#pragma endregion
	
#pragma region Chamber states	

	UE_DEFINE_GAMEPLAY_TAG(Chamber_Empty, "VRModularWeaponSystem.ChamberState.Empty");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_RoundReady, "VRModularWeaponSystem.ChamberState.RoundReady");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_SpentCasing, "VRModularWeaponSystem.ChamberState.SpentCasing");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_Jammed, "VRModularWeaponSystem.ChamberState.Jammed");
	
#pragma endregion
	
#pragma region Mechanical movement
	
	UE_DEFINE_GAMEPLAY_TAG(Linear, "VRModularWeaponSystem.MechanicalMovement.Linear");
	UE_DEFINE_GAMEPLAY_TAG(Rotational, "VRModularWeaponSystem.MechanicalMovement.Rotational");

#pragma endregion
}
