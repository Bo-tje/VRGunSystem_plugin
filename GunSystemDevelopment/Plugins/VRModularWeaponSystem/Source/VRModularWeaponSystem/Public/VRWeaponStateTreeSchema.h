#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionTypes.h"
#include "Components/StateTreeComponentSchema.h"
#include "VRWeaponStateTreeSchema.generated.h"

UCLASS(BlueprintType, EditInlineNew, meta=(DisplayName="VR Weapon State Tree Schema"))
class VRMODULARWEAPONSYSTEM_API UVRWeaponStateTreeSchema : public UStateTreeComponentSchema
{
	GENERATED_BODY()
	
public:
	UVRWeaponStateTreeSchema();

protected:
	UPROPERTY(EditAnywhere, Category="Context")
	FStateTreeExternalDataDesc WeaponDataDescription;
};
