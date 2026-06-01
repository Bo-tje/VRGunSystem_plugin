#pragma once

#include "NativeGameplayTags.h"

namespace  VRNativeTags
{
#pragma region Input actions
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trigger);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryInput);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryInput_Left);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(PrimaryInput_Right);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SecondaryInput);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SecondaryInput_Left);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(SecondaryInput_Right);

#pragma endregion
	
#pragma region Chamber states	

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_Empty);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_RoundReady);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_SpentCasing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_Jammed);
	
#pragma endregion
	
#pragma region Mechanical movement
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Mechanical_Max);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Mechanical_Min);
	
#pragma endregion

#pragma region Magazine types

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(MagazineType);

#pragma endregion
	
};

