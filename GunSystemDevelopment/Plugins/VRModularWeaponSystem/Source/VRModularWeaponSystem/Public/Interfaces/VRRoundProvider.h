#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRRoundProvider.generated.h"

class UProjectileData;

UINTERFACE(MinimalAPI, BlueprintType)
class UVRRoundProvider : public UInterface { GENERATED_BODY() };

/**
 * Interface for any component that can provide a round to a FireComponent.
 */
class VRMODULARWEAPONSYSTEM_API IVRRoundProvider
{
	GENERATED_BODY()

public:
	/** 
	 * Attempts to provide a round for firing. 
	 * Returns true if a round was successfully provided.
	 * @param OutRound The projectile data to be fired.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon")
	bool GetRound(UProjectileData*& OutRound);
};
