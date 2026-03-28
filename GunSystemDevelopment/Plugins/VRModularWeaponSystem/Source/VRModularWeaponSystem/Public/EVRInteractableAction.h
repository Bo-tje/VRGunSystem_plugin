#pragma once

#include "CoreMinimal.h"
#include "EVRInteractableAction.generated.h"

UENUM(BlueprintType)
enum class EVRInteractableAction : uint8
{
	Action1     UMETA(DisplayName = "Action 1"),
	Action2     UMETA(DisplayName = "Action 2"),
	Action3     UMETA(DisplayName = "Action 3"),
	Action4     UMETA(DisplayName = "Action 4"),
};
