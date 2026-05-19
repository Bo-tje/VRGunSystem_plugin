#include "Components/VRMagwellComponent.h"
#include "Core/VRMagazineBase.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRWeaponFeedbackComponent.h"
#include "Data/VRWeaponData.h"
#include "Data/MagazineData.h"
#include "Interaction/VRGrabComponent.h"
#include "Interaction/VRInteractor.h"
#include "Kismet/GameplayStatics.h"

UVRMagwellComponent::UVRMagwellComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetCollisionProfileName("Trigger");
	InitSphereRadius(InsertRadius);
}

void UVRMagwellComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OnComponentBeginOverlap.AddDynamic(this, &UVRMagwellComponent::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this, &UVRMagwellComponent::OnOverlapEnd);
}

void UVRMagwellComponent::InitializeComponent_Implementation(UVRWeaponData* InData)
{
	// Base initialization if needed
}

void UVRMagwellComponent::InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings)
{
	if (UVRMagwellSettings* MagwellSettings = Cast<UVRMagwellSettings>(InSettings))
	{
		MagazineSocketName = MagwellSettings->MagazineSocketName;
		CompatibleMagazinesTag = MagwellSettings->CompatibleMagazinesTag;
		bEjectOnRelease = MagwellSettings->bEjectOnRelease;
		InsertRadius = MagwellSettings->InsertRadius;
		InsertSound = MagwellSettings->InsertSound;
		EjectSound = MagwellSettings->EjectSound;
		HoverHapticEffect = MagwellSettings->HoverHapticEffect;
		InsertHapticEffect = MagwellSettings->InsertHapticEffect;
		EjectHapticEffect = MagwellSettings->EjectHapticEffect;

		SetSphereRadius(InsertRadius);

		// Attach the drop zone to a specific socket on the weapon mesh if defined
		if (!MagwellSettings->DropZoneSocketName.IsNone())
		{
			if (AActor* Owner = GetOwner())
			{
				USceneComponent* AttachTarget = nullptr;
				TArray<UMeshComponent*> MeshComponents;
				Owner->GetComponents(MeshComponents);
				for (UMeshComponent* MeshComp : MeshComponents)
				{
					if (MeshComp->DoesSocketExist(MagwellSettings->DropZoneSocketName))
					{
						AttachTarget = MeshComp;
						break;
					}
				}

				if (AttachTarget)
				{
					AttachToComponent(AttachTarget, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MagwellSettings->DropZoneSocketName);
				}
			}
		}
	}
}

bool UVRMagwellComponent::GetRound_Implementation(UProjectileData*& OutRound)
{
	if (AttachedMagazine)
	{
		return IVRRoundProvider::Execute_GetRound(AttachedMagazine, OutRound);
	}

	OutRound = nullptr;
	return false;
}

void UVRMagwellComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AttachedMagazine || !OtherActor) return;

	AVRMagazineBase* Magazine = Cast<AVRMagazineBase>(OtherActor);
	if (Magazine && Magazine->MagazineData)
	{
		// Check compatibility
		if (CompatibleMagazinesTag.IsValid() && Magazine->MagazineData->MagazineType != CompatibleMagazinesTag)
		{
			return; // Not compatible
		}

		// ONLY allow interaction/snapping if the player is actually holding the magazine!
		if (!Magazine->GrabComponent || !Magazine->GrabComponent->IsHeld())
		{
			return;
		}

		OverlappingMagazine = Magazine;

		// Play hover haptic pulse on the hand holding the magazine to signal entry
		if (UVRInteractor* Interactor = Magazine->GrabComponent->GetCurrentInteractor())
		{
			if (HoverHapticEffect)
			{
				Interactor->PlayHapticFeedback(HoverHapticEffect, 1.0f, false);
			}
		}

		// Bind to the release event. When the user lets go, it snaps into place.
		Magazine->GrabComponent->OnGrabReleased.RemoveDynamic(this, &UVRMagwellComponent::OnMagazineReleased);
		Magazine->GrabComponent->OnGrabReleased.AddDynamic(this, &UVRMagwellComponent::OnMagazineReleased);
	}
}

void UVRMagwellComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == OverlappingMagazine)
	{
		if (OverlappingMagazine && OverlappingMagazine->GrabComponent)
		{
			OverlappingMagazine->GrabComponent->OnGrabReleased.RemoveDynamic(this, &UVRMagwellComponent::OnMagazineReleased);
		}
		OverlappingMagazine = nullptr;
	}
}

void UVRMagwellComponent::OnMagazineGrabbed(AActor* InteractingHand)
{
	if (AttachedMagazine)
	{
		if (bEjectOnRelease)
		{
			EjectMagazine();
		}
	}
}

void UVRMagwellComponent::OnMagazineReleased()
{
	if (!OverlappingMagazine) return;

	// Snap into place
	AttachedMagazine = OverlappingMagazine;
	OverlappingMagazine = nullptr;

	if (AttachedMagazine->GrabComponent)
	{
		AttachedMagazine->GrabComponent->OnGrabReleased.RemoveDynamic(this, &UVRMagwellComponent::OnMagazineReleased);
		
		// Bind to grabbed so we can detach it
		AttachedMagazine->GrabComponent->OnGrabbed.RemoveDynamic(this, &UVRMagwellComponent::OnMagazineGrabbed);
		AttachedMagazine->GrabComponent->OnGrabbed.AddDynamic(this, &UVRMagwellComponent::OnMagazineGrabbed);

		// Disable physics simulation and physical blocking to prevent physics explosions
		AttachedMagazine->GrabComponent->SetSimulatePhysics(false);
		AttachedMagazine->GrabComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// Attach to socket
	if (AActor* Owner = GetOwner())
	{
		// Explicitly ignore collision between the gun and the magazine using primitive components
		if (UPrimitiveComponent* WeaponRoot = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
		{
			WeaponRoot->IgnoreActorWhenMoving(AttachedMagazine, true);
		}
		AttachedMagazine->GrabComponent->IgnoreActorWhenMoving(Owner, true);

		USceneComponent* OwnerRoot = Owner->GetRootComponent();
		if (OwnerRoot && OwnerRoot->DoesSocketExist(MagazineSocketName))
		{
			AttachedMagazine->AttachToComponent(OwnerRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale, MagazineSocketName);
		}
		else
		{
			AttachedMagazine->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}

		// Play Insertion Sound
		if (InsertSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, InsertSound, GetComponentLocation());
		}

		// Play Insertion Haptics on the weapon itself to vibrate the hand holding the weapon
		if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(Owner))
		{
			if (Weapon->FeedbackComponent && InsertHapticEffect)
			{
				Weapon->FeedbackComponent->PlayFiringFeedback(InsertHapticEffect, 1.0f);
			}
		}
	}

	OnMagazineAttached.Broadcast(AttachedMagazine);
}

void UVRMagwellComponent::EjectMagazine()
{
	if (AttachedMagazine)
	{
		if (AttachedMagazine->GrabComponent)
		{
			AttachedMagazine->GrabComponent->OnGrabbed.RemoveDynamic(this, &UVRMagwellComponent::OnMagazineGrabbed);

			// Enable physics if it's not currently held
			if (!AttachedMagazine->GrabComponent->IsHeld())
			{
				AttachedMagazine->GrabComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				AttachedMagazine->GrabComponent->SetSimulatePhysics(true);
			}
		}

		AttachedMagazine->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		
		// Restore collision between gun and magazine
		if (AActor* Owner = GetOwner())
		{
			if (UPrimitiveComponent* WeaponRoot = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
			{
				WeaponRoot->IgnoreActorWhenMoving(AttachedMagazine, false);
			}
			AttachedMagazine->GrabComponent->IgnoreActorWhenMoving(Owner, false);
		}

		// Play Eject Sound
		if (EjectSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EjectSound, GetComponentLocation());
		}

		// Play Eject Haptics on the weapon
		if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(GetOwner()))
		{
			if (Weapon->FeedbackComponent && EjectHapticEffect)
			{
				Weapon->FeedbackComponent->PlayFiringFeedback(EjectHapticEffect, 1.0f);
			}
		}

		AVRMagazineBase* DetachedMag = AttachedMagazine;
		AttachedMagazine = nullptr;

		OnMagazineDetached.Broadcast(DetachedMag);
	}
}
