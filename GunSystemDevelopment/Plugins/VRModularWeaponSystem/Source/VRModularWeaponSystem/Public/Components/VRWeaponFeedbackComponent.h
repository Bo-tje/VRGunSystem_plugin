#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRWeaponFeedbackComponent.generated.h"

class AVRWeaponBase;
class UVRInteractor;

/**
 * Centralized component for weapon haptics and audio feedback.
 * Handles different feedback intensities for primary vs secondary holding hands.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRMODULARWEAPONSYSTEM_API UVRWeaponFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVRWeaponFeedbackComponent();

	/** Plays haptics on all holding interactors with intelligent scaling based on grip role. */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Feedback")
	void PlayFiringFeedback(UHapticFeedbackEffect_Base* HapticEffect, float BaseScale = 1.0f);

protected:
	UPROPERTY()
	TObjectPtr<AVRWeaponBase> WeaponOwner;

	virtual void BeginPlay() override;

private:
	/** Returns the interactor that is considered the 'Primary' hand (usually the one on the main grip). */
	UVRInteractor* GetPrimaryInteractor() const;
};
