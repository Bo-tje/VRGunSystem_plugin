#pragma once

#include "CoreMinimal.h"
#include "VRWeaponBase.h"
#include "StateTreeEvaluatorBase.h"
#include "VRChamberComponent.h"
#include "StateTreeWeaponEvaluator.generated.h"

USTRUCT(BlueprintType)
struct VRMODULARWEAPONSYSTEM_API FSTEval_WeaponInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bIsTriggerPulled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	bool bHasRoundReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	FGameplayTag ChamberStateTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Output")
	float TimeBetweenShots = 0.1f; 
};

USTRUCT(meta = (DisplayName = "Weapon Evaluator", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTEval_Weapon : public FStateTreeEvaluatorBase
{
	GENERATED_BODY()

	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTEval_WeaponInstanceData::StaticStruct(); }
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

