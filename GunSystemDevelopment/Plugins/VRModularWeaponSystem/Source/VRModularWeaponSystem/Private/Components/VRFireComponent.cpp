#include "Components/VRFireComponent.h"
#include "Data/ProjectileData.h"
#include "Data/VRWeaponData.h"
#include "Data/VRWeaponStats.h"
#include "Interfaces/VRRoundProvider.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRWeaponFeedbackComponent.h"
#include "Interaction/VRInteractor.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Core/VRProjectileBase.h"

UVRFireComponent::UVRFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	FireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComponent"));
	FireAudioComponent->SetupAttachment(this);
	FireAudioComponent->bAutoActivate = false;

	DryFireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DryFireAudioComponent"));
	DryFireAudioComponent->SetupAttachment(this);
	DryFireAudioComponent->bAutoActivate = false;
}

void UVRFireComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	WeaponData = InData;
	
	if (FireAudioComponent && !FireAudioComponent->IsRegistered())
	{
		FireAudioComponent->RegisterComponent();
	}

	if (DryFireAudioComponent && !DryFireAudioComponent->IsRegistered())
	{
		DryFireAudioComponent->RegisterComponent();
	}
	
	if (WeaponData)
	{
		if (WeaponData->FireSound) FireAudioComponent->SetSound(WeaponData->FireSound);
		if (WeaponData->DryFireSound) DryFireAudioComponent->SetSound(WeaponData->DryFireSound);
	}
}

void UVRFireComponent::InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings)
{
	InitializeComponent_Implementation(InData);

	if (UVRFireSettings* FireSettings = Cast<UVRFireSettings>(InSettings))
	{
		MuzzleSocketName = FireSettings->MuzzleSocketName;
		FireHapticScale = FireSettings->FireHapticScale;
		DryFireHapticScale = FireSettings->DryFireHapticScale;
	}
}

FTransform UVRFireComponent::GetMuzzleTransform() const
{
	AActor* MyOwner = GetOwner();
	if (!MyOwner) return GetComponentTransform();
	
	USceneComponent* Root = MyOwner->GetRootComponent();
	if (Root && Root->DoesSocketExist(MuzzleSocketName))
	{
		return Root->GetSocketTransform(MuzzleSocketName);
	}
	
	return GetComponentTransform();
}

void UVRFireComponent::OnRegister()
{
	Super::OnRegister();
}

void UVRFireComponent::PullTrigger_Implementation()
{
	// Trigger logic handled by StateTree
}

void UVRFireComponent::ReleaseTrigger_Implementation()
{
}

void UVRFireComponent::HandleFiring(UProjectileData* ProjectileData)
{
	AVRWeaponBase* WeaponOwner = Cast<AVRWeaponBase>(GetOwner());
	if (!WeaponOwner || !WeaponData) return;

	// 1. Get Calculated Stats (including attachment modifiers)
	FVRWeaponStats Stats = WeaponOwner->GetCalculatedStats();

	// 2. Determine Projectile
	UProjectileData* FinalProjectile = ProjectileData ? ProjectileData : WeaponData->DefaultProjectile.Get();
	if (!FinalProjectile) return;
	
	const FTransform MuzzleTransform = GetMuzzleTransform();
	
	int32 BasePellets = FinalProjectile->PelletCount;
	int32 TotalPellets = FMath::Max(1, BasePellets + Stats.PelletCountOffset);
	float BaseSpread = FinalProjectile->SpreadAngle;
	float CurrentSpreadAngle = BaseSpread * Stats.SpreadMultiplier;

	for (int32 i = 0; i < TotalPellets; ++i)
	{
		FRotator FinalRotation = MuzzleTransform.GetRotation().Rotator();
		if (CurrentSpreadAngle > 0.01f)
		{
			float HalfAngleRad = FMath::DegreesToRadians(CurrentSpreadAngle / 2.0f);
			FVector RandomDir = FMath::VRandCone(MuzzleTransform.GetRotation().GetForwardVector(), HalfAngleRad);
			FinalRotation = RandomDir.Rotation();
		}

		FTransform PelletTransform(FinalRotation, MuzzleTransform.GetLocation(), MuzzleTransform.GetScale3D());

		// 3. Logic - Spawn Projectile / Hitscan
		if (WeaponData->bUseHitscan)
		{
			PerformHitscan(FinalProjectile, PelletTransform.GetLocation(), PelletTransform.GetRotation().Rotator());
		}
		else if (FinalProjectile->ProjectileClass)
		{
			AVRProjectileBase::SpawnProjectileFromData(
				this, 
				FinalProjectile, 
				PelletTransform, 
				GetOwner(), 
				GetOwner() ? GetOwner()->GetInstigator() : nullptr
			);
		}
	}

	if (WeaponData->bAutoPlayWeaponFeedback)
	{
		// 4. Feedback - Use Centralized System
		if (WeaponOwner->FeedbackComponent && WeaponData->FireHapticEffect)
		{
			WeaponOwner->FeedbackComponent->PlayFiringFeedback(WeaponData->FireHapticEffect, FireHapticScale);
		}

		// 5. Visuals / Audio
		if (FireAudioComponent)
		{
			if (Stats.FireSoundOverride) FireAudioComponent->SetSound(Stats.FireSoundOverride);
			FireAudioComponent->Play();
		}

		if (Stats.MuzzleFlashOverride)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Stats.MuzzleFlashOverride, MuzzleTransform);
		}
		else if (WeaponData->MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponData->MuzzleFlash, MuzzleTransform);
		}
	}
	

	// Procedural Recoil Impulse
	if (WeaponOwner)
	{
		float PitchImpulse = Stats.RecoilPitch * Stats.RecoilMultiplier;
		float YawImpulse = FMath::RandRange(-Stats.RecoilYaw, Stats.RecoilYaw) * Stats.RecoilMultiplier;
		WeaponOwner->TargetRecoilOffset += FRotator(PitchImpulse, YawImpulse, 0.0f);
	}

	OnFired.Broadcast();

}

void UVRFireComponent::HandleDryFire() 
{
	AVRWeaponBase* WeaponOwner = Cast<AVRWeaponBase>(GetOwner());
	if (!WeaponOwner) return;

	if (WeaponData && WeaponData->bAutoPlayWeaponFeedback)
	{
		if (DryFireAudioComponent) DryFireAudioComponent->Play();

		// Centralized Feedback
		if (WeaponOwner->FeedbackComponent && WeaponData->FireHapticEffect)
		{
			WeaponOwner->FeedbackComponent->PlayFiringFeedback(WeaponData->FireHapticEffect, DryFireHapticScale);
		}
	}

	OnDryFired.Broadcast();
}

void UVRFireComponent::PlayHaptics(UHapticFeedbackEffect_Base* HapticEffect, float InHapticScale)
{
	if (AVRWeaponBase* WeaponOwner = Cast<AVRWeaponBase>(GetOwner()))
	{
		if (WeaponOwner->FeedbackComponent)
		{
			WeaponOwner->FeedbackComponent->PlayFiringFeedback(HapticEffect, InHapticScale);
		}
	}
}

void UVRFireComponent::PerformHitscan(const UProjectileData* Data, const FVector& StartLocation, const FRotator& StartRotation) const
{
	FHitResult HitResult; 
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FVector LineTraceEnd = StartLocation + (StartRotation.Vector() * Data->HitscanRange);

	bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		StartLocation, 
		LineTraceEnd, 
		ECC_Visibility, 
		QueryParams
	);
	
	DrawDebugLine(GetWorld(), StartLocation, bIsHit ? HitResult.Location : LineTraceEnd, FColor::Red, false, 2.0f);
    
	if (bIsHit)
	{
		if (Data->ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), Data->ImpactSound, HitResult.Location);
		}

		if (AActor* HitActor = HitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitActor, Data->HitscanDamage, nullptr, GetOwner(), UDamageType::StaticClass());
		}
	}
}
