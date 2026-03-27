#pragma once

#include "CoreMinimal.h"
#include "EVRChamberStatus.generated.h"

UENUM(BlueprintType)
enum class EVRChamberStatus : uint8
{
	Empty         UMETA(DisplayName = "Empty"),
	RoundReady    UMETA(DisplayName = "Round Ready"),   
	SpentCasing   UMETA(DisplayName = "Spent Casing"),  
	DoubleFeed    UMETA(DisplayName = "Double Feed")
};