#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponent.h"
#include "VRWeaponStateTreeComponent.generated.h"

/**
 * Specialized StateTree component for VR Weapons.
 * Handles automatic binding of weapon-specific context data.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRMODULARWEAPONSYSTEM_API UVRWeaponStateTreeComponent : public UStateTreeComponent
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<UStateTreeSchema> GetSchema() const override;

protected:
	/** UE 5.6: Overridden to manually bind the WeaponData context. */
	virtual bool SetContextRequirements(FStateTreeExecutionContext& Context, bool bLogErrors) override;
};
