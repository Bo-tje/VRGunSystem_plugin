#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VRWeaponComponentInterface.generated.h"

class UVRWeaponData;
class UVRWeaponComponentSettings;
class UVRWeaponStatModifier;

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
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon Component")
	void InitializeComponent(UVRWeaponData* InData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon Component")
	void InitializeComponentWithSettings(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings);

	/** Returns the stat modifier this component provides, if any. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VR Weapon Component")
	UVRWeaponStatModifier* GetStatModifier() const;
};
