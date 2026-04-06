#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRWeaponComponentInterface.generated.h"

class UVRWeaponData;

UINTERFACE(MinimalAPI)
class UVRWeaponComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for components that belong to a VR Weapon and need to be initialized with WeaponData.
 */
class VRMODULARWEAPONSYSTEM_API IVRWeaponComponentInterface
{
	GENERATED_BODY()

public:
	/** Called by the Weapon Base to pass data to the component. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon Component")
	void InitializeComponent(UVRWeaponData* InData);
};
