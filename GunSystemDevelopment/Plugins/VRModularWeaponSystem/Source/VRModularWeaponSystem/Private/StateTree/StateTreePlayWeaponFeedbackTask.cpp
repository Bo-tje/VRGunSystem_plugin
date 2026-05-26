#include "StateTree/StateTreePlayWeaponFeedbackTask.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRFireComponent.h"
#include "Components/VRWeaponFeedbackComponent.h"
#include "Data/VRWeaponData.h"
#include "Kismet/GameplayStatics.h"
#include "Core/VRWeaponBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

EStateTreeRunStatus FSTTask_PlayWeaponFeedback::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_PlayWeaponFeedbackInstanceData& InstanceData = Context.GetInstanceData<FSTTask_PlayWeaponFeedbackInstanceData>(*this);
	
	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	if (!Weapon || !Weapon->WeaponData) return EStateTreeRunStatus::Failed;

	UVRFireComponent* FireComponent = Weapon->CachedFireComponent;
	FTransform MuzzleTransform = FireComponent ? FireComponent->GetMuzzleTransform() : Weapon->GetActorTransform();
	
	FVRWeaponStats Stats = Weapon->GetCalculatedStats();

	// 1. Play Sound
	USoundBase* SoundToPlay = SoundOverride;
	if (!SoundToPlay)
	{
		if (bIsReload)
		{
			SoundToPlay = Weapon->WeaponData->ReloadSound;
		}
		else if (bIsDryFire)
		{
			SoundToPlay = Weapon->WeaponData->DryFireSound;
		}
		else
		{
			SoundToPlay = Stats.FireSoundOverride ? Stats.FireSoundOverride : Weapon->WeaponData->FireSound;
		}
	}
	
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(Weapon, SoundToPlay, MuzzleTransform.GetLocation());
	}

	// 2. Play Haptics
	UHapticFeedbackEffect_Base* HapticToPlay = HapticOverride;
	if (!HapticToPlay)
	{
		HapticToPlay = Weapon->WeaponData->FireHapticEffect;
	}

	if (HapticToPlay && Weapon->FeedbackComponent)
	{
		float Scale = HapticScaleOverride;
		if (HapticOverride == nullptr)
		{
			// Fallback to FireComponent scales if we are using default haptics
			if (FireComponent)
			{
				Scale = bIsDryFire ? FireComponent->DryFireHapticScale : FireComponent->FireHapticScale;
			}
		}
		Weapon->FeedbackComponent->PlayFiringFeedback(HapticToPlay, Scale);
	}

	// 3. Play Muzzle Flash (only if not dry firing and not reloading)
	if (!bIsDryFire && !bIsReload)
	{
		UNiagaraSystem* FlashToPlay = Stats.MuzzleFlashOverride ? Stats.MuzzleFlashOverride : Weapon->WeaponData->MuzzleFlash;
		if (FlashToPlay)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(Weapon->GetWorld(), FlashToPlay, MuzzleTransform.GetLocation(), MuzzleTransform.GetRotation().Rotator());
		}
	}

	return EStateTreeRunStatus::Succeeded;
}
