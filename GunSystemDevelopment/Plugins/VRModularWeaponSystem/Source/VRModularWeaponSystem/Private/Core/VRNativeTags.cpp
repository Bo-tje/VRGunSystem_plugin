#include "Core/VRNativeTags.h"


namespace VRNativeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Trigger, "VRModularWeaponSystem.Interaction.Trigger");
	UE_DEFINE_GAMEPLAY_TAG(TriggerReleased, "VRModularWeaponSystem.Interaction.TriggerReleased");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInput, "VRModularWeaponSystem.Interaction.PrimaryInput");
	UE_DEFINE_GAMEPLAY_TAG(PrimaryInputReleased, "VRModularWeaponSystem.Interaction.PrimaryInputReleased");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInput, "VRModularWeaponSystem.Interaction.SecondaryInput");
	UE_DEFINE_GAMEPLAY_TAG(SecondaryInputReleased, "VRModularWeaponSystem.Interaction.SecondaryInputReleased");
	UE_DEFINE_GAMEPLAY_TAG(Reload, "VRModularWeaponSystem.Interaction.Reload");
	UE_DEFINE_GAMEPLAY_TAG(ReloadReleased, "VRModularWeaponSystem.Interaction.ReloadReleased");

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
}
