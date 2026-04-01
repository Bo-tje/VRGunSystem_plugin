#pragma once

#include "NativeGameplayTags.h"

namespace  VRNativeTags
{
#pragma region Input actions
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trigger);

#pragma endregion
	
#pragma region Weapon states
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Held);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Firing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Reloading);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LockedBack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Jammed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Empty);
	
#pragma endregion
	
#pragma region Chamber states	

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_Empty);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_RoundReady);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_SpentCasing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Chamber_Jammed);
	
#pragma endregion
};
