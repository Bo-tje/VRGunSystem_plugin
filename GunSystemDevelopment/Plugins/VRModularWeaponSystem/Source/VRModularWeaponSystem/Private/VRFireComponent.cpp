#include "VRFireComponent.h"

#include "ProjectileData.h"
#include "VRWeaponData.h"
#include "VRRoundProvider.h"
#include "VRWeaponBase.h"
#include "VRInteractor.h"
#include "Kismet/GameplayStatics.h"

UVRFireComponent::UVRFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRFireComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	WeaponData = InData;
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

void UVRFireComponent::HandleFiring(UProjectileData* ProjectileData) const
{
	if (!ProjectileData) return;
	
	const FTransform SpawnTransform = GetMuzzleTransform();
	const FVector MuzzleLocation = SpawnTransform.GetLocation();
	const FRotator MuzzleRotation = SpawnTransform.GetRotation().Rotator();
	
	if (WeaponData->bUseHitscan)
	{
		PerformHitscan(ProjectileData, MuzzleLocation, MuzzleRotation);
	}
	else if (ProjectileData->ProjectileClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = GetOwner()->GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(ProjectileData->ProjectileClass, SpawnTransform, SpawnParams);
	}
	
	if (WeaponData->MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponData->MuzzleFlash, SpawnTransform);
	}
	
	if (WeaponData->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, MuzzleLocation);
	}

	if (WeaponData->FireHapticEffect)
	{
		if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(GetOwner()))
		{
			if (UVRInteractor* Interactor = Weapon->GetHoldingInteractor())
			{
				if (APlayerController* PC = Interactor->GetProvidingPlayerController())
				{
					PC->PlayHapticEffect(WeaponData->FireHapticEffect, Interactor->HandSide);
				}
			}
		}
	}
	OnFired.Broadcast();
}

void UVRFireComponent::HandleDryFire() const
{
	if (WeaponData && WeaponData->DryFireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->DryFireSound, GetMuzzleTransform().GetLocation());
	}

	OnDryFired.Broadcast();
}

void UVRFireComponent::SpawnProjectile(UProjectileData* ProjectileData)
{
	// Will implement custom spawning logic here later
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
