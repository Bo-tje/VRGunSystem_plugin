#include "VRWeaponBase.h"
#include "VRWeaponData.h"
#include "VRWeaponComponentInterface.h"
#include "VRGrabComponent.h"
#include "VRNativeTags.h"
#include "Components/StaticMeshComponent.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	RootComponent = WeaponRoot;

	PartRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PartRoot"));
	PartRoot->SetupAttachment(RootComponent);
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

	ClearOldParts();

	// Local cache to keep track of components we just created for efficient searching
	TArray<USceneComponent*> CreatedComponents;
	CreatedComponents.Add(PartRoot);

	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		// 1. Handle Logic Components
		if (Part.LogicComponentClass)
		{
			UActorComponent* NewLogic = NewObject<UActorComponent>(this, Part.LogicComponentClass);
			if (NewLogic)
			{
				if (USceneComponent* SceneComp = Cast<USceneComponent>(NewLogic))
				{
					bool bAttached = false;
					if (!Part.ParentSocketName.IsNone())
					{
						// Search only the components we've created in this loop
						for (USceneComponent* PotentialParent : CreatedComponents)
						{
							if (PotentialParent->DoesSocketExist(Part.ParentSocketName))
							{
								SceneComp->AttachToComponent(PotentialParent, FAttachmentTransformRules::SnapToTargetIncludingScale, Part.ParentSocketName);
								bAttached = true;
								break;
							}
						}
					}
					if (!bAttached) SceneComp->AttachToComponent(PartRoot, FAttachmentTransformRules::SnapToTargetIncludingScale);
					CreatedComponents.Add(SceneComp);
				}
				NewLogic->RegisterComponent();
			}
			continue;
		}

		// 2. Handle Static Parts
		if (!Part.Mesh) continue;

		FName ComponentName = FName(*FString::Printf(TEXT("MeshPart_%s"), *Part.PartName));
		UStaticMeshComponent* NewMesh = NewObject<UStaticMeshComponent>(this, ComponentName);
		
		if (NewMesh)
		{
			NewMesh->SetStaticMesh(Part.Mesh);
			NewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			bool bAttached = false;
			if (!Part.ParentSocketName.IsNone())
			{
				for (USceneComponent* PotentialParent : CreatedComponents)
				{
					if (PotentialParent->DoesSocketExist(Part.ParentSocketName))
					{
						NewMesh->AttachToComponent(PotentialParent, FAttachmentTransformRules::SnapToTargetIncludingScale, Part.ParentSocketName);
						bAttached = true;
						break;
					}
				}
			}

			if (!bAttached) NewMesh->AttachToComponent(PartRoot, FAttachmentTransformRules::SnapToTargetIncludingScale);

			NewMesh->RegisterComponent();
			
			// Cache it
			CreatedComponents.Add(NewMesh);
			PartMeshes.Add(Part.PartSlotTag, NewMesh);
		}
	}
}

void AVRWeaponBase::ClearOldParts()
{
	// Destroy any dynamic static meshes
	for (auto& Elem : PartMeshes)
	{
		if (Elem.Value) Elem.Value->DestroyComponent();
	}
	PartMeshes.Empty();

	// Also clear logic components if needed, though they aren't tracked in PartMeshes
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
