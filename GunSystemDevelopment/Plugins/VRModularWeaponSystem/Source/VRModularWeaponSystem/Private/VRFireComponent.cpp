#include "VRFireComponent.h"
#include "VRWeaponData.h"
#include "VRRoundProvider.h"
#include "Kismet/GameplayStatics.h"

UVRFireComponent::UVRFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRFireComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	WeaponData = InData;
}

void UVRFireComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!MuzzleLocation)
	{
		MuzzleLocation = NewObject<USceneComponent>(GetOwner(), TEXT("MuzzleLocation"));
		MuzzleLocation->RegisterComponent();
		MuzzleLocation->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void UVRFireComponent::OnRegister()
{
	Super::OnRegister();
}

void UVRFireComponent::PullTrigger_Implementation()
{
	HandleFiring(WeaponData->DefaultProjectile);
}

void UVRFireComponent::ReleaseTrigger_Implementation()
{
}

void UVRFireComponent::HandleFiring(UProjectileData* ProjectileData)
{
	if (!WeaponData) return;

	bool bFoundProvider = false;
	bool bHasRound = false;

	// 1. Check for a Round Provider (Chamber, Mag, etc.)
	TArray<UActorComponent*> ProviderComponents;
	GetOwner()->GetComponents(ProviderComponents);

	for (UActorComponent* Component : ProviderComponents)
	{
		if (Component && Component->Implements<UVRRoundProvider>())
		{
			bFoundProvider = true;
			UProjectileData* DummyRound = nullptr;
			if (IVRRoundProvider::Execute_GetRound(Component, DummyRound))
			{
				bHasRound = true;
				break; 
			}
			else
			{
				// Found a provider, but it's empty (Dry Fire)
				if (WeaponData->DryFireSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, WeaponData->DryFireSound, GetComponentLocation());
				}
				OnDryFired.Broadcast();
				return;
			}
		}
	}

	// 2. Determine if we should fire (If provider has round, OR if no provider exists [Arcade Mode])
	if (bHasRound || !bFoundProvider)
	{
		// Play Firing Sound
		if (WeaponData->FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, GetComponentLocation());
		}

		// Play Muzzle Flash
		if (WeaponData->MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponData->MuzzleFlash, MuzzleLocation->GetComponentTransform());
		}
		
		if (WeaponData->bUseHitscan)
		{
			PerformHitscan(WeaponData);
		}
		

		OnFired.Broadcast();
	}
}

void UVRFireComponent::SpawnProjectile(UProjectileData* ProjectileData)
{
	// User will implement custom spawning logic here later
}

void UVRFireComponent::PerformHitscan(const UVRWeaponData* Data) const
{
	
	FVector StartLocation = this->GetComponentLocation();
	FRotator StartRotation = this->GetComponentRotation();
	
	FHitResult HitResult; 
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FVector LineTraceEnd = StartLocation + StartRotation.Vector() * WeaponData->HitscanRange;
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitResult, 
		StartLocation, 
		LineTraceEnd, 
		ECC_Visibility, QueryParams);
	
	DrawDebugLine(GetWorld(), StartLocation, LineTraceEnd, FColor::Red, false, 2.0f);
	
	if (bIsHit)
	{
		LineTraceEnd = HitResult.Location;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->HitscanImpactSound, HitResult.Location);

		if (AActor* HitActor = HitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitActor, WeaponData->HitscanDamage, nullptr, GetOwner(), UDamageType::StaticClass());
		}
	}
}
