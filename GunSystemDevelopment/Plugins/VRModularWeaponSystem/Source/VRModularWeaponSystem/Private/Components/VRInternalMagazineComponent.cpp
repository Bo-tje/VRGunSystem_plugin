#include "Components/VRInternalMagazineComponent.h"
#include "Components/SphereComponent.h"
#include "Core/VRRoundActor.h"
#include "Core/VRWeaponBase.h"
#include "Interaction/VRGrabComponent.h"
#include "Interaction/VRInteractor.h"
#include "Data/ProjectileData.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UVRInternalMagazineComponent::UVRInternalMagazineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	LoadDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LoadDetectionSphere"));
	LoadDetectionSphere->SetupAttachment(this);
	LoadDetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
	LoadDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LoadDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	LoadDetectionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	LoadDetectionSphere->SetGenerateOverlapEvents(true);
}

void UVRInternalMagazineComponent::BeginPlay()
{
	Super::BeginPlay();

	if (LoadDetectionSphere)
	{
		LoadDetectionSphere->SetSphereRadius(LoadRadius);
		LoadDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UVRInternalMagazineComponent::OnOverlapBegin);
	}
}

void UVRInternalMagazineComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	// Standard scene component initialization
}

bool UVRInternalMagazineComponent::GetRound_Implementation(UProjectileData*& OutRound)
{
	if (LoadedRounds.Num() > 0)
	{
		OutRound = LoadedRounds[0];
		LoadedRounds.RemoveAt(0);
		OnAmmoChanged.Broadcast(LoadedRounds.Num());
		return true;
	}
	OutRound = nullptr;
	return false;
}

void UVRInternalMagazineComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsFull())
	{
		return;
	}

	AVRRoundActor* RoundActor = Cast<AVRRoundActor>(OtherActor);
	if (!RoundActor || !RoundActor->ProjectileData)
	{
		return;
	}

	// 1. Verify compatible ammo tags
	if (CompatibleAmmoTag.IsValid() && !RoundActor->ProjectileData->AmmoTags.HasTag(CompatibleAmmoTag))
	{
		return;
	}

	// 2. Verify required weapon state tags
	if (RequiredWeaponStateTags.Num() > 0)
	{
		AVRWeaponBase* WeaponBase = Cast<AVRWeaponBase>(GetOwner());
		if (!WeaponBase || !WeaponBase->HasAllStateTags(RequiredWeaponStateTags))
		{
			return;
		}
	}

	// 3. Insert round
	LoadedRounds.Add(RoundActor->ProjectileData);
	OnAmmoChanged.Broadcast(LoadedRounds.Num());

	UVRGrabComponent* RoundGrab = RoundActor->GrabComponent;
	UVRInteractor* HoldingInteractor = nullptr;
	if (RoundGrab)
	{
		HoldingInteractor = RoundGrab->GetCurrentInteractor();
		RoundGrab->TryRelease();
	}

	// Play sound
	if (LoadSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LoadSound, GetComponentLocation());
	}

	// Play haptics
	if (HoldingInteractor && LoadHaptic)
	{
		HoldingInteractor->PlayHapticFeedback(LoadHaptic, 1.0f, false);
	}

	// Destroy the physical round actor
	RoundActor->Destroy();
}
