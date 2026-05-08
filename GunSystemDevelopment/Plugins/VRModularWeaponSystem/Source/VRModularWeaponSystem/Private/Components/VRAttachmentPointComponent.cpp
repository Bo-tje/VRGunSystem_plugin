#include "Components/VRAttachmentPointComponent.h"
#include "Core/VRAttachmentActor.h"
#include "Core/VRWeaponBase.h"

UVRAttachmentPointComponent::UVRAttachmentPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	InitSphereRadius(10.0f);
	SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void UVRAttachmentPointComponent::BeginPlay()
{
	Super::BeginPlay();
	OnComponentBeginOverlap.AddDynamic(this, &UVRAttachmentPointComponent::OnOverlapBegin);
}

void UVRAttachmentPointComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AVRAttachmentActor* Attachment = Cast<AVRAttachmentActor>(OtherActor))
	{
		// Only attach if released
		if (!Attachment->bIsHeld)
		{
			TryAttach(Attachment);
		}
	}
}

bool UVRAttachmentPointComponent::TryAttach(AVRAttachmentActor* AttachmentActor)
{
	if (CurrentAttachment || !AttachmentActor) return false;
	if (AttachmentActor->AttachmentSocketName != SocketName && AttachmentActor->AttachmentSocketName != NAME_None) return false;

	CurrentAttachment = AttachmentActor;

	// Snap and weld
	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	CurrentAttachment->AttachToComponent(this, AttachRules);
	CurrentAttachment->MeshComponent->SetSimulatePhysics(false);
	CurrentAttachment->MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));

	// Recalculate weapon stats
	if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(GetOwner()))
	{
		Weapon->UpdateCalculatedStats();

		// Apply modifiers manually since UpdateCalculatedStats only checks CachedWeaponComponents
		if (CurrentAttachment->StatModifier)
		{
			Weapon->CalculatedStats.FireRate += CurrentAttachment->StatModifier->FireRateOffset;
			Weapon->CalculatedStats.RecoilMultiplier *= CurrentAttachment->StatModifier->RecoilMultiplier;
			Weapon->CalculatedStats.DamageMultiplier *= CurrentAttachment->StatModifier->DamageMultiplier;
			Weapon->CalculatedStats.BulletVelocityMultiplier *= CurrentAttachment->StatModifier->BulletVelocityMultiplier;
			Weapon->CalculatedStats.ReloadSpeedMultiplier *= CurrentAttachment->StatModifier->ReloadSpeedMultiplier;

			if (CurrentAttachment->StatModifier->MuzzleFlashOverride) Weapon->CalculatedStats.MuzzleFlashOverride = CurrentAttachment->StatModifier->MuzzleFlashOverride;
			if (CurrentAttachment->StatModifier->FireSoundOverride) Weapon->CalculatedStats.FireSoundOverride = CurrentAttachment->StatModifier->FireSoundOverride;
		}
	}

	return true;
}

void UVRAttachmentPointComponent::Detach()
{
	if (!CurrentAttachment) return;

	CurrentAttachment->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentAttachment->MeshComponent->SetSimulatePhysics(true);
	CurrentAttachment->MeshComponent->SetCollisionProfileName(TEXT("PhysicsBody"));

	CurrentAttachment = nullptr;

	if (AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(GetOwner()))
	{
		Weapon->UpdateCalculatedStats();
	}
}
