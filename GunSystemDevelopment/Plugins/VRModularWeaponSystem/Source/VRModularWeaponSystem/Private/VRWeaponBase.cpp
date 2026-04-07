#include "VRWeaponBase.h"
#include "VRWeaponData.h"
#include "VRWeaponComponentInterface.h"
#include "VRGrabComponent.h"
#include "VRNativeTags.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRoot"));
	WeaponRoot->SetBoxExtent(FVector(20.0f, 5.0f, 15.0f));
	
	// FIX: Force collision to be compatible with physics simulation
	WeaponRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponRoot->SetCollisionProfileName(TEXT("PhysicsBody"));
	
	WeaponRoot->SetSimulatePhysics(true);
	RootComponent = WeaponRoot;
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

	// Find the Grab Component
	GrabComponent = FindComponentByClass<UVRGrabComponent>();
	if (GrabComponent)
	{
		GrabComponent->OnGrabbed.AddDynamic(this, &AVRWeaponBase::OnGrabbed);
		GrabComponent->OnReleased.AddDynamic(this, &AVRWeaponBase::OnReleased);
		GrabComponent->StartAction.AddDynamic(this, &AVRWeaponBase::HandleActionStart);
		GrabComponent->StopAction.AddDynamic(this, &AVRWeaponBase::HandleActionStop);
	}
	
	InitializeWeapon();
}

void AVRWeaponBase::InitializeWeapon()
{
	if (!WeaponData) return;

	TArray<UActorComponent*> WeaponComponents;
	GetComponents(WeaponComponents);

	for (UActorComponent* Component : WeaponComponents)
	{
		if (Component && Component->Implements<UVRWeaponComponentInterface>())
		{
			IVRWeaponComponentInterface::Execute_InitializeComponent(Component, WeaponData);
		}
	}
}

void AVRWeaponBase::ApplyWeaponDataVisuals()
{
	if (!WeaponData) return;

	TArray<UStaticMeshComponent*> Components;
	GetComponents(Components);

	// Match existing components in the BP by name to the entries in WeaponData
	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (Part.PartName.IsNone() || !Part.Mesh) continue;

		for (UStaticMeshComponent* Comp : Components)
		{
			// BP component names often contain the variable name.
			if (Comp->GetName().Contains(Part.PartName.ToString()))
			{
				Comp->SetStaticMesh(Part.Mesh);
				break;
			}
		}
	}
}

void AVRWeaponBase::OnGrabbed(AActor* InteractingHand) {}
void AVRWeaponBase::OnReleased() {}

void AVRWeaponBase::HandleActionStart(UObject* Interactor, float Value, FGameplayTag ActionTag)
{
	if (ActionTag.MatchesTag(VRNativeTags::Trigger))
	{
		IVRWeaponInterface::Execute_PullTrigger(this);
	}
}

void AVRWeaponBase::HandleActionStop(UObject* Interactor, FGameplayTag ActionTag)
{
	if (ActionTag.MatchesTag(VRNativeTags::Trigger))
	{
		IVRWeaponInterface::Execute_ReleaseTrigger(this);
	}
}

void AVRWeaponBase::PullTrigger_Implementation()
{
	TArray<UActorComponent*> TriggerComponents;
	GetComponents(TriggerComponents);
	for (UActorComponent* Component : TriggerComponents)
	{
		if (Component && Component->Implements<UVRWeaponInterface>())
		{
			IVRWeaponInterface::Execute_PullTrigger(Component);
		}
	}
}

void AVRWeaponBase::ReleaseTrigger_Implementation()
{
	TArray<UActorComponent*> TriggerComponents;
	GetComponents(TriggerComponents);
	for (UActorComponent* Component : TriggerComponents)
	{
		if (Component && Component->Implements<UVRWeaponInterface>())
		{
			IVRWeaponInterface::Execute_ReleaseTrigger(Component);
		}
	}
}
