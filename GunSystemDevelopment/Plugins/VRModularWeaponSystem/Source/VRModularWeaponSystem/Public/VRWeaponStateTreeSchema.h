#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeSchema.h"
#include "VRWeaponStateTreeSchema.generated.h"


UCLASS(BlueprintType, EditInlineNew, meta=(DisplayName="VR Weapon State Tree Schema"))
class VRMODULARWEAPONSYSTEM_API UVRWeaponStateTreeSchema : public UStateTreeSchema
{
	GENERATED_BODY()
	
	public:
	UVRWeaponStateTreeSchema();
	
	protected:

	UPROPERTY(EditAnywhere, Category="Context")
	FStateTreeExternalDataDesc WeaponActorDescription;
	
	UPROPERTY(EditAnywhere, Category="Context")
	FStateTreeExternalDataDesc WeaponDataDescription;
};
