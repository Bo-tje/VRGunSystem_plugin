#pragma once

#include "CoreMinimal.h" 
#include "EVRWeaponStates.generated.h"

UENUM(BlueprintType) 
enum class EVRWeaponStates : uint8 
{
	Idle          UMETA(DisplayName = "Idle"),
	Held          UMETA(DisplayName = "Held"),
	Firing        UMETA(DisplayName = "Firing"),
	Reloading     UMETA(DisplayName = "Reloading"),
	LockedBack    UMETA(DisplayName = "Locked Back"),
	Jammed        UMETA(DisplayName = "Jammed"),
};