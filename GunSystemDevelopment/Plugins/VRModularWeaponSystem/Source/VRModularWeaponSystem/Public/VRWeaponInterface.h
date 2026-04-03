#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRWeaponInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UVRWeaponInterface : public UInterface { GENERATED_BODY() };

class VRMODULARWEAPONSYSTEM_API IVRWeaponInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon")
	void PullTrigger();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon")
	void ReleaseTrigger();
};
