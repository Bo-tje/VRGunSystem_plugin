#include "StateTree/StateTreePlayWeaponFeedbackTask.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRFireComponent.h"
#include "Components/VRWeaponFeedbackComponent.h"
#include "Data/VRWeaponData.h"
#include "Kismet/GameplayStatics.h"

EStateTreeRunStatus FSTTask_PlayWeaponFeedback::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTTask_PlayWeaponFeedbackInstanceData& InstanceData = Context.GetInstanceData<FSTTask_PlayWeaponFeedbackInstanceData>(*this);
	
	if (!InstanceData.WeaponActor) return EStateTreeRunStatus::Failed;
	
	AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
	if (!Weapon || !Weapon->WeaponData) return EStateTreeRunStatus::Failed;

	UVRFireComponent* FireComponent = Weapon->FindComponentByClass<UVRFireComponent>();
	FTransform MuzzleTransform = FireComponent ? FireComponent->GetMuzzleTransform() : Weapon->GetActorTransform();
	
	FVRWeaponStats Stats = Weapon->GetCalculatedStats();

	// 1. Play Sound
	USoundBase* SoundToPlay = SoundOverride;
	if (!SoundToPlay)
	{
		if (bIsDryFire)
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

	// 3. Play Muzzle Flash (only if not dry firing)
	if (!bIsDryFire)
	{
		UParticleSystem* FlashToPlay = Stats.MuzzleFlashOverride ? Stats.MuzzleFlashOverride : Weapon->WeaponData->MuzzleFlash;
		if (FlashToPlay)
		{
			UGameplayStatics::SpawnEmitterAtLocation(Weapon->GetWorld(), FlashToPlay, MuzzleTransform);
		}
	}

	return EStateTreeRunStatus::Succeeded;
}
