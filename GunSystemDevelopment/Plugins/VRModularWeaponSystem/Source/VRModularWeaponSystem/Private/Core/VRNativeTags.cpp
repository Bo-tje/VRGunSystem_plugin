#include "Core/VRNativeTags.h"


namespace VRNativeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Trigger, "VRModularWeaponSystem.Interaction.Trigger");
	
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInput, "VRModularWeaponSystem.Interaction.PrimaryInput");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInput_Left, "VRModularWeaponSystem.Interaction.PrimaryInput.Left");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInput_Right, "VRModularWeaponSystem.Interaction.PrimaryInput.Right");
	
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInput, "VRModularWeaponSystem.Interaction.SecondaryInput");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInput_Left, "VRModularWeaponSystem.Interaction.SecondaryInput.Left");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInput_Right, "VRModularWeaponSystem.Interaction.SecondaryInput.Right");

#pragma endregion
	
#pragma region Chamber states	

	UE_DEFINE_GAMEPLAY_TAG(Chamber_Empty, "VRModularWeaponSystem.ChamberState.Empty");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_RoundReady, "VRModularWeaponSystem.ChamberState.RoundReady");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_SpentCasing, "VRModularWeaponSystem.ChamberState.SpentCasing");
	UE_DEFINE_GAMEPLAY_TAG(Chamber_Jammed, "VRModularWeaponSystem.ChamberState.Jammed");
	
#pragma endregion
	
#pragma region Mechanical movement
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Mechanical_Max, "VRModularWeaponSystem.Event.Mechanical.Max");
	UE_DEFINE_GAMEPLAY_TAG(Event_Mechanical_Min, "VRModularWeaponSystem.Event.Mechanical.Min");

#pragma endregion

#pragma region Magazine types

	UE_DEFINE_GAMEPLAY_TAG(MagazineType, "VRModularWeaponSystem.MagazineType");

#pragma endregion
}
