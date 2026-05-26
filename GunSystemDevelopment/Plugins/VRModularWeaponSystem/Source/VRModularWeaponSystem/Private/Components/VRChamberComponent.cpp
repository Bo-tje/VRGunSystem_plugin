#include "Components/VRChamberComponent.h"
#include "Core/VRNativeTags.h"
#include "Data/ProjectileData.h"
#include "Data/VRWeaponData.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Core/VREjectedCasing.h"
#include "Core/VRRoundActor.h"
#include "Core/VRWeaponBase.h"
#include "Interaction/VRGrabComponent.h"
#include "Interaction/VRInteractor.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealObjectPooler.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UVRChamberComponent::UVRChamberComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentChamberState = VRNativeTags::Chamber_Empty;
	LoadedProjectile = nullptr;
	
	RoundVisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoundVisualMesh"));
	RoundVisualMesh->SetupAttachment(this);
	
	//Disable collision so the round doesn't fight with the weapon physics body
	RoundVisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RoundVisualMesh->SetCollisionProfileName(TEXT("NoCollision"));
	RoundVisualMesh->SetGenerateOverlapEvents(false);

	LoadDetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LoadDetectionSphere"));
	LoadDetectionSphere->SetupAttachment(this);
	LoadDetectionSphere->SetCollisionProfileName(TEXT("Trigger"));
	LoadDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LoadDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	LoadDetectionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	LoadDetectionSphere->SetGenerateOverlapEvents(true);
}

void UVRChamberComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	WeaponData = InData;

	if (RoundVisualMesh && !RoundVisualMesh->IsRegistered())
	{
		RoundVisualMesh->RegisterComponent();
	}
}

bool UVRChamberComponent::GetRound_Implementation(UProjectileData*& OutRound)
{
	OutRound = LoadedProjectile;
	return TryGiveBullet();
}

bool UVRChamberComponent::HasRound_Implementation() const
{
	return IsRoundReady();
}

void UVRChamberComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateVisuals();

	if (LoadDetectionSphere)
	{
		LoadDetectionSphere->SetSphereRadius(ManualLoadRadius);
		LoadDetectionSphere->OnComponentBeginOverlap.AddDynamic(this, &UVRChamberComponent::OnOverlapBegin);
	}
}

bool UVRChamberComponent::TryGiveBullet()
{
	if (IsRoundReady())
	{
		if (FMath::FRand() <= JamChance)
		{
			SetChamberState(VRNativeTags::Chamber_Jammed);
			return false;
		}

		OnRoundFired.Broadcast(LoadedProjectile);
		
		SetChamberState(VRNativeTags::Chamber_SpentCasing);
		return true;
	}
	return false;
}

bool UVRChamberComponent::TryLoad(UProjectileData* NewRound)
{
	if (!NewRound) return false;

	if (IsEmpty())
	{
		LoadedProjectile = NewRound;
		SetChamberState(VRNativeTags::Chamber_RoundReady);
		OnRoundLoaded.Broadcast(LoadedProjectile);
		return true;
	}

	return false;
}

UProjectileData* UVRChamberComponent::TryEject()
{
	if (IsEmpty() && !LoadedProjectile) return nullptr;

	UProjectileData* EjectedRound = LoadedProjectile;
	FGameplayTag PreviousChamberState = CurrentChamberState;
	
	if (EjectedRound)
	{
		OnRoundEjected.Broadcast(EjectedRound);

		if (PreviousChamberState == VRNativeTags::Chamber_SpentCasing)
		{
			SpawnChamberSmoke();
		}

		UStaticMesh* CasingMesh = (PreviousChamberState == VRNativeTags::Chamber_SpentCasing) ? EjectedRound->SpentCasingMesh : EjectedRound->LiveRoundMesh;

		if (CasingMesh)
		{
			if (UWorld* World = GetWorld())
			{
				FTransform SpawnTransform = GetComponentTransform();
				TSubclassOf<AVREjectedCasing> SpawnClass = EjectedCasingClass ? EjectedCasingClass : TSubclassOf<AVREjectedCasing>(AVREjectedCasing::StaticClass());
				
				AVREjectedCasing* SpawnedCasing = nullptr;

				if (UUnrealObjectPooler* Pooler = World->GetSubsystem<UUnrealObjectPooler>())
				{
					SpawnedCasing = Pooler->SpawnObject<AVREjectedCasing>(SpawnClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
					if (SpawnedCasing)
					{
						SpawnedCasing->SetOwner(GetOwner());
						SpawnedCasing->SetInstigator(GetOwner() ? GetOwner()->GetInstigator() : nullptr);
					}
				}
				else
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = GetOwner();
					SpawnParams.Instigator = GetOwner() ? GetOwner()->GetInstigator() : nullptr;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					SpawnedCasing = World->SpawnActor<AVREjectedCasing>(SpawnClass, SpawnTransform, SpawnParams);
				}

				if (SpawnedCasing)
				{
					if (BounceSoundsOverride.Num() > 0)
					{
						SpawnedCasing->BounceSounds = BounceSoundsOverride;
					}
					else if (EjectedRound->ImpactSound)
					{
						SpawnedCasing->BounceSounds.Empty();
						SpawnedCasing->BounceSounds.Add(EjectedRound->ImpactSound);
					}

					FVector WorldEjectDir = GetComponentTransform().TransformVectorNoScale(EjectVelocityDirection);
					FVector EjectImpulse = WorldEjectDir * EjectVelocityStrength;

					if (AActor* WeaponOwner = GetOwner())
					{
						EjectImpulse += WeaponOwner->GetVelocity();
					}

					SpawnedCasing->InitializeCasing(CasingMesh, EjectImpulse);
				}
			}
		}
	}
	
	LoadedProjectile = nullptr;
	SetChamberState(VRNativeTags::Chamber_Empty);

	return EjectedRound;
}

bool UVRChamberComponent::IsRoundReady() const
{
	return CurrentChamberState == VRNativeTags::Chamber_RoundReady && LoadedProjectile != nullptr;
}

bool UVRChamberComponent::IsEmpty() const
{
	return CurrentChamberState == VRNativeTags::Chamber_Empty;
}

void UVRChamberComponent::SetChamberState(FGameplayTag NewState)
{
	if (CurrentChamberState != NewState)
	{
		CurrentChamberState = NewState;
		UpdateVisuals();
		OnChamberStateChanged.Broadcast(CurrentChamberState);
	}
}

void UVRChamberComponent::UpdateVisuals()
{
	if (!RoundVisualMesh) return;

	if (IsEmpty())
	{
		RoundVisualMesh->SetVisibility(false);
	}
	else
	{
		RoundVisualMesh->SetVisibility(true);
		if (LoadedProjectile)
		{
			if (CurrentChamberState == VRNativeTags::Chamber_SpentCasing && LoadedProjectile->SpentCasingMesh)
			{
				RoundVisualMesh->SetStaticMesh(LoadedProjectile->SpentCasingMesh);
			}
			else if (LoadedProjectile->LiveRoundMesh)
			{
				RoundVisualMesh->SetStaticMesh(LoadedProjectile->LiveRoundMesh);
			}
		}
	}
}

void UVRChamberComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bAllowManualLoading || !IsEmpty())
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

	// 3. Try to chamber the round
	if (TryLoad(RoundActor->ProjectileData))
	{
		UVRGrabComponent* RoundGrab = RoundActor->GrabComponent;
		UVRInteractor* HoldingInteractor = nullptr;
		if (RoundGrab)
		{
			HoldingInteractor = RoundGrab->GetCurrentInteractor();
			RoundGrab->TryRelease();
		}

		// Play sound
		if (ManualLoadSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ManualLoadSound, GetComponentLocation());
		}

		// Play haptics
		if (HoldingInteractor && ManualLoadHaptic)
		{
			HoldingInteractor->PlayHapticFeedback(ManualLoadHaptic, ManualLoadHapticScale, false);
		}

		// Destroy the physical round actor
		RoundActor->Destroy();
	}
}

void UVRChamberComponent::SpawnChamberSmoke()
{
	UNiagaraSystem* SmokeToSpawn = ChamberSmokeNiagara ? ChamberSmokeNiagara : (WeaponData ? WeaponData->ChamberSmoke : nullptr);
	if (SmokeToSpawn)
	{
		if (UWorld* World = GetWorld())
		{
			FTransform SpawnTransform = GetChamberSmokeTransform();
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, SmokeToSpawn, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
		}
	}
}

FTransform UVRChamberComponent::GetChamberSmokeTransform() const
{
	FTransform ComponentTransform = GetComponentTransform();
	FVector WorldOffset = ComponentTransform.TransformVectorNoScale(ChamberSmokeOffset);
	FQuat WorldRotation = ComponentTransform.GetRotation() * ChamberSmokeRotation.Quaternion();
	
	return FTransform(WorldRotation, ComponentTransform.GetLocation() + WorldOffset);
}
