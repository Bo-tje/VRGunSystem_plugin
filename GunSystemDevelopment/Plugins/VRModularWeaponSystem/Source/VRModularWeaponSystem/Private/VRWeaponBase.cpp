#include "VRWeaponBase.h"
#include "VRWeaponData.h"
#include "VRWeaponComponentInterface.h"
#include "VRGrabComponent.h"
#include "VRNativeTags.h"
#include "VRWeaponStateTreeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRoot"));
	WeaponRoot->SetBoxExtent(FVector(20.0f, 5.0f, 15.0f));

	WeaponRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponRoot->SetCollisionProfileName(TEXT("PhysicsBody"));

	WeaponRoot->SetSimulatePhysics(true);
	RootComponent = WeaponRoot;

	StateTreeComponent = CreateDefaultSubobject<UVRWeaponStateTreeComponent>(TEXT("StateTree"));
}

void AVRWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	ApplyWeaponDataVisuals();
	InitializeWeapon();
}

void AVRWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UVRGrabComponent* GrabComponent = FindComponentByClass<UVRGrabComponent>())
	{
		GrabComponent->OnGrabbed.AddDynamic(this, &AVRWeaponBase::OnGrabbed);
		GrabComponent->OnReleased.AddDynamic(this, &AVRWeaponBase::OnReleased);
	}
	
	InitializeWeapon();
}

void AVRWeaponBase::InitializeWeapon()
{
	if (!WeaponData) return;
	
	CachedWeaponComponents.Empty();
	CachedInputComponents.Empty();

	TArray<UActorComponent*> AllComponents;
	GetComponents(AllComponents);

	for (UActorComponent* Component : AllComponents)
	{
		if (Component->Implements<UVRWeaponComponentInterface>())
		{
			CachedWeaponComponents.Add(Component);
			IVRWeaponComponentInterface::Execute_InitializeComponent(Component, WeaponData);
		}

		if (Component->Implements<UVRWeaponInterface>())
		{
			CachedInputComponents.Add(Component);
		}
	}
}

void AVRWeaponBase::ApplyWeaponDataVisuals()
{
	if (!WeaponData) return;

	TArray<UStaticMeshComponent*> Components;
	GetComponents(Components);
	
	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (Part.PartName.IsNone() || !Part.Mesh) continue;

		for (UStaticMeshComponent* Component : Components)
		{
			if (Component->GetName().Contains(Part.PartName.ToString()))
			{
				Component->SetStaticMesh(Part.Mesh);
				break;
			}
		}
	}
}

void AVRWeaponBase::OnGrabbed(AActor* InteractingHand) {}
void AVRWeaponBase::OnReleased() {}

void AVRWeaponBase::StartAction_Implementation(UObject* Interactor, float ActionValue, FGameplayTag ActionTag)
{
	if (ActionTag.MatchesTag(VRNativeTags::Trigger))
	{
		IVRWeaponInterface::Execute_PullTrigger(this);
	}
	else if (ActionTag.MatchesTag(VRNativeTags::PrimaryInput))
	{
		IVRWeaponInterface::Execute_PrimaryAction(this);
	}
	else if (ActionTag.MatchesTag(VRNativeTags::SecondaryInput))
	{
		IVRWeaponInterface::Execute_SecondaryAction(this);
	}
}

void AVRWeaponBase::StopAction_Implementation(UObject* Interactor, FGameplayTag ActionTag)
{
	if (ActionTag.MatchesTag(VRNativeTags::Trigger) || ActionTag.MatchesTag(VRNativeTags::TriggerReleased))
	{
		IVRWeaponInterface::Execute_ReleaseTrigger(this);
	}
	else if (ActionTag.MatchesTag(VRNativeTags::PrimaryInput) || ActionTag.MatchesTag(VRNativeTags::PrimaryInputReleased))
	{
		IVRWeaponInterface::Execute_ReleasePrimaryAction(this);
	}
	else if (ActionTag.MatchesTag(VRNativeTags::SecondaryInput) || ActionTag.MatchesTag(VRNativeTags::SecondaryInputReleased))
	{
		IVRWeaponInterface::Execute_ReleaseSecondaryAction(this);
	}
}

UVRInteractor* AVRWeaponBase::GetHoldingInteractor() const
{
	if (UVRGrabComponent* GC = FindComponentByClass<UVRGrabComponent>())
	{
		return GC->GetCurrentInteractor();
	}
	return nullptr;
}

void AVRWeaponBase::PullTrigger_Implementation()
{


	if (StateTreeComponent)
	{
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::Trigger);
	}
}

void AVRWeaponBase::ReleaseTrigger_Implementation()
{


	if (StateTreeComponent)
	{
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::TriggerReleased);
	}
}

void AVRWeaponBase::PrimaryAction_Implementation()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::PrimaryInput);
	}
}

void AVRWeaponBase::ReleasePrimaryAction_Implementation()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::PrimaryInputReleased);
	}
}

void AVRWeaponBase::SecondaryAction_Implementation()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::SecondaryInput);
	}
}

void AVRWeaponBase::ReleaseSecondaryAction_Implementation()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::SecondaryInputReleased);
	}
}

bool AVRWeaponBase::IsTriggerPulled_Implementation() const
{
	return bIsTriggerPulled;
}

void AVRWeaponBase::SetWeaponState_Implementation(FGameplayTag NewState)
{
	CurrentWeaponState = NewState;
}
