#include "VRFireComponent.h"
#include "VRWeaponData.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UVRFireComponent::UVRFireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRFireComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVRFireComponent::OnRegister()
{
	Super::OnRegister();
}

void UVRFireComponent::InitializeComponent(UVRWeaponData* InData)
{
	WeaponData = InData;
}

/*
void UVRFireComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UVRFireComponent, WeaponData))
	{
		InitializeWeapon();
	}
}
#endif
*/

void UVRFireComponent::HandleFiring()
{
	if (WeaponData->bUseHitscan)
	{
		PerformHitscan(WeaponData);
	}
}

void UVRFireComponent::PerformHitscan(const UVRWeaponData* ProjectileInfo) const
{
	FVector StartLocation = this->GetComponentLocation();
	FRotator StartRotation = this->GetComponentRotation();
	
	FHitResult HitResult; 
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FVector LineTraceEnd = StartLocation + StartRotation.Vector() * ProjectileInfo->HitscanRange;
	bool bIsHit = GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		StartLocation, 
		LineTraceEnd, 
		ECC_Visibility, QueryParams);
	
	DrawDebugLine(GetWorld(), StartLocation, LineTraceEnd, FColor::Red, false, 2.0f);
	if (bIsHit)
	{
		LineTraceEnd = HitResult.Location;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileInfo->HitscanImpactSound, HitResult.Location);

		if (AActor* HitActor = HitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitActor, ProjectileInfo->HitscanDamage, nullptr, GetOwner(), UDamageType::StaticClass());
		}
	}
}

void UVRFireComponent::PullTrigger_Implementation()
{
	
}

void UVRFireComponent::ReleaseTrigger_Implementation()
{
	
}

