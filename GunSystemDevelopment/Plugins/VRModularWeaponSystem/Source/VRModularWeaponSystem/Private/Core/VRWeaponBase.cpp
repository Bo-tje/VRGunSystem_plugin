#include "Core/VRWeaponBase.h"
#include "Data/VRWeaponData.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interaction/VRGrabComponent.h"
#include "Core/VRNativeTags.h"
#include "Interfaces/VRRoundProvider.h"
#include "Components/VRWeaponStateTreeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRoot"));
	// Shrink the root to a tiny 1x1x1 core so it doesn't cause clipping
	WeaponRoot->SetBoxExtent(FVector(1.0f, 1.0f, 1.0f));

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
	
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("Init"));
	}
	
	TArray<UVRGrabComponent*> AllGrabComponents;
	GetComponents(AllGrabComponents);
	for (UVRGrabComponent* GrabComponent : AllGrabComponents)
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
	CachedRoundProviders.Empty();

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

		if (Component->Implements<UVRRoundProvider>())
		{
			CachedRoundProviders.Add(Component);
		}
	}
}

void AVRWeaponBase::ApplyWeaponDataVisuals()
{
	if (!WeaponData) return;
	
	// --- Process Visual Parts ---
	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (Part.PartName.IsNone() || Part.Mesh.IsNull()) continue;

		// Dynamically create the component based on the socket system
		UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this, Part.PartName);
		if (NewComp)
		{
			NewComp->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			NewComp->SetStaticMesh(Part.Mesh.LoadSynchronous());
			
			// Determine what to attach to based on the socket name
			USceneComponent* AttachTarget = WeaponRoot;
			if (!Part.ParentSocket.IsNone())
			{
				// Search existing components for one that has this socket
				TArray<UStaticMeshComponent*> TrackedComps;
				GetComponents(TrackedComps);
				for (UStaticMeshComponent* MC : TrackedComps)
				{
					if (MC->DoesSocketExist(Part.ParentSocket))
					{
						AttachTarget = MC;
						break;
					}
				}
			}

			NewComp->SetupAttachment(AttachTarget, Part.ParentSocket);
			NewComp->RegisterComponent();
			NewComp->SetRelativeTransform(Part.PartOffset);
			
			// Enable collision so Unreal welds this shape to the Root physics body
			NewComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			NewComp->SetCollisionProfileName(TEXT("PhysicsBody"));
		}
	}

	// --- Process Dynamic Components (Grab points, logic, etc.) ---
	for (const FVRWeaponDynamicComponent& CompGen : WeaponData->AdditionalComponents)
	{
		if (!CompGen.ComponentClass) continue;

		UActorComponent* NewObj = NewObject<UActorComponent>(this, CompGen.ComponentClass, CompGen.ComponentName);
		if (NewObj)
		{
			NewObj->CreationMethod = EComponentCreationMethod::UserConstructionScript;

			// If it is a SceneComponent, we might need to attach it
			if (USceneComponent* SceneComp = Cast<USceneComponent>(NewObj))
			{
				USceneComponent* AttachTarget = WeaponRoot;
				
				if (!CompGen.ParentSocket.IsNone())
				{
					TArray<UStaticMeshComponent*> TrackedComps;
					GetComponents(TrackedComps);
					for (UStaticMeshComponent* MC : TrackedComps)
					{
						if (MC->DoesSocketExist(CompGen.ParentSocket))
						{
							AttachTarget = MC;
							break;
						}
					}
				}

				SceneComp->SetupAttachment(AttachTarget, CompGen.ParentSocket);
				SceneComp->RegisterComponent();
				SceneComp->SetRelativeTransform(CompGen.RelativeOffset);
			}
			else
			{
				// It's a pure logic component, just register it
				NewObj->RegisterComponent();
			}
		}
	}
}

void AVRWeaponBase::OnGrabbed(AActor* InteractingHand)
{
	if (StateTreeComponent)
	{
		StateTreeComponent->SetComponentTickEnabled(true);
		StateTreeComponent->StartLogic();
	}
}

void AVRWeaponBase::OnReleased()
{
	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("Released"));
	}
	
	// Reset trigger state if released
	if (bIsTriggerPulled)
	{
		IVRWeaponInterface::Execute_ReleaseTrigger(this);
	}
}

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

TArray<UVRInteractor*> AVRWeaponBase::GetHoldingInteractors() const
{
	TArray<UVRInteractor*> ActiveInteractors;
	
	for (UVRGrabComponent* GrabComponent : CachedGrabComponents)
	{
		if (GrabComponent->IsHeld() && GrabComponent->GetCurrentInteractor())
		{
			ActiveInteractors.Add(GrabComponent->GetCurrentInteractor());
		}
	}
	
	return ActiveInteractors;
}

UVRInteractor* AVRWeaponBase::GetHoldingInteractor() const
{
	TArray<UVRInteractor*> ActiveInteractors = GetHoldingInteractors();
	return ActiveInteractors.Num() > 0 ? ActiveInteractors[0] : nullptr;
}



void AVRWeaponBase::PullTrigger_Implementation()
{
	if (bIsTriggerPulled) return;
	
	bIsTriggerPulled = true;

	if (StateTreeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Trigger recieved"));
		StateTreeComponent->SendStateTreeEvent(VRNativeTags::Trigger);
	}
}

void AVRWeaponBase::ReleaseTrigger_Implementation()
{
	if (!bIsTriggerPulled) return;
	
	bIsTriggerPulled = false;

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
