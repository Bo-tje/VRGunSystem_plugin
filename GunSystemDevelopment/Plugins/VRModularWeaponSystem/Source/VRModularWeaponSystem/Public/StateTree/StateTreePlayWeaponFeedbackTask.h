#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Core/VRWeaponBase.h"
#include "StateTreePlayWeaponFeedbackTask.generated.h"

USTRUCT(meta=(DisplayName = "Play Weapon Feedback Instance Data"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_PlayWeaponFeedbackInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> WeaponActor = nullptr;
};

USTRUCT(meta=(DisplayName = "Play Weapon Feedback", Category = "Weapon"))
struct VRMODULARWEAPONSYSTEM_API FSTTask_PlayWeaponFeedback : public FStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual const UScriptStruct* GetInstanceDataType() const override { return FSTTask_PlayWeaponFeedbackInstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
protected:
	/** If true, this will attempt to play the Dry Fire sounds and haptics instead of regular fire. */
	UPROPERTY(EditAnywhere, Category = "Parameter") 
	bool bIsDryFire = false;

	/** If set, overrides the sound from the weapon data. */
	UPROPERTY(EditAnywhere, Category = "Overrides")
	TObjectPtr<USoundBase> SoundOverride = nullptr;

	/** If set, overrides the haptics from the weapon data. */
	UPROPERTY(EditAnywhere, Category = "Overrides")
	TObjectPtr<UHapticFeedbackEffect_Base> HapticOverride = nullptr;
	
	/** The scale of the haptic effect if overridden. */
	UPROPERTY(EditAnywhere, Category = "Overrides")
	float HapticScaleOverride = 1.0f;
};
