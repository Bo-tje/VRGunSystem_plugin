#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRInteractorInterface.generated.h"

UINTERFACE(MinimalAPI)
class UVRInteractorInterface : public UInterface { GENERATED_BODY() };

class IVRInteractorInterface
{
    GENERATED_BODY()

public:
    virtual APlayerController* GetProvidingPlayerController() const = 0;
};