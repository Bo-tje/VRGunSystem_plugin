#include "Components/VRWeaponFeedbackComponent.h"
#include "Core/VRWeaponBase.h"
#include "Interaction/VRInteractor.h"
#include "Interaction/VRGrabComponent.h"

UVRWeaponFeedbackComponent::UVRWeaponFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRWeaponFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();
	WeaponOwner = Cast<AVRWeaponBase>(GetOwner());
}

void UVRWeaponFeedbackComponent::PlayFiringFeedback(UHapticFeedbackEffect_Base* HapticEffect, float BaseScale)
{
	if (!WeaponOwner) return;

	// Inject Procedural Recoil Kick
	FVRWeaponStats Stats = WeaponOwner->GetCalculatedStats();
	float Pitch = Stats.RecoilPitch * Stats.RecoilMultiplier;
	float Yaw = FMath::RandRange(-Stats.RecoilYaw, Stats.RecoilYaw) * Stats.RecoilMultiplier;
	
	WeaponOwner->TargetRecoilOffset += FRotator(Pitch, Yaw, 0.0f);

	if (!HapticEffect) return;

	TArray<UVRInteractor*> Interactors = WeaponOwner->GetHoldingInteractors();
	UVRInteractor* Primary = GetPrimaryInteractor();

	for (UVRInteractor* Interactor : Interactors)
	{
		if (!Interactor) continue;

		float Scale = BaseScale;
		
		// If this is the secondary hand (foregrip), reduce haptic intensity
		if (Interactor != Primary)
		{
			Scale *= 0.4f; // 40% intensity for supporting hand
		}

		Interactor->PlayHapticFeedback(HapticEffect, Scale);
	}
}

UVRInteractor* UVRWeaponFeedbackComponent::GetPrimaryInteractor() const
{
	if (!WeaponOwner) return nullptr;
	
	// Implementation detail: The 'Primary' hand is usually the first interactor to grab the weapon,
	// or specifically the one holding the main grip component.
	// Check if any active grab component is marked as the main grip
	TArray<UVRGrabComponent*> GrabComponents;
	WeaponOwner->GetComponents(GrabComponents);
	for (UVRGrabComponent* GrabComp : GrabComponents)
	{
		if (GrabComp->IsHeld() && GrabComp->bIsMainGrip)
		{
			return GrabComp->GetCurrentInteractor();
		}
	}
	
	// Fallback: assume the first interactor is primary
	return WeaponOwner->GetHoldingInteractor();
}
