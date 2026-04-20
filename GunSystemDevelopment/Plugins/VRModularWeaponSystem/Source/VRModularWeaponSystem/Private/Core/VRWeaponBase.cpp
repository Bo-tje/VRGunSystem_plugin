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
		GrabComponent->OnGrabReleased.AddDynamic(this, &AVRWeaponBase::OnReleased);
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
	
	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (Part.PartName.IsNone() || Part.Mesh.IsNull()) continue;
		
		UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(this, Part.PartName);
		if (NewComponent)
		{
			NewComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			NewComponent->SetStaticMesh(Part.Mesh.LoadSynchronous());
			
			USceneComponent* AttachTarget = WeaponRoot;
			if (!Part.ParentSocket.IsNone())
			{
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

			NewComponent->SetupAttachment(AttachTarget, Part.ParentSocket);
			NewComponent->RegisterComponent();
			NewComponent->SetRelativeTransform(Part.PartOffset);
			
			NewComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			NewComponent->SetCollisionProfileName(TEXT("PhysicsBody"));
		}
	}
	
	for (const FVRWeaponDynamicComponent& GeneratedComponent : WeaponData->AdditionalComponents)
	{
		if (!GeneratedComponent.ComponentClass) continue;

		UActorComponent* NewObj = NewObject<UActorComponent>(this, GeneratedComponent.ComponentClass, GeneratedComponent.ComponentName);
		if (NewObj)
		{
			NewObj->CreationMethod = EComponentCreationMethod::UserConstructionScript;

			if (USceneComponent* SceneComp = Cast<USceneComponent>(NewObj))
			{
				USceneComponent* AttachTarget = WeaponRoot;

				if (!GeneratedComponent.ParentSocket.IsNone())
				{
					TArray<UStaticMeshComponent*> TrackedComps;
					GetComponents(TrackedComps);
					for (UStaticMeshComponent* MC : TrackedComps)
					{
						if (MC->DoesSocketExist(GeneratedComponent.ParentSocket))
						{
							AttachTarget = MC;
							break;
						}
					}
				}

				if (UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(NewObj))
				{
					if (!GeneratedComponent.OptionalMesh.IsNull())
					{
						SMComp->SetStaticMesh(GeneratedComponent.OptionalMesh.LoadSynchronous());
						if (GeneratedComponent.bWeldCollision)
						{
							SMComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
							SMComp->SetCollisionProfileName(TEXT("PhysicsBody"));
						}
					}
				}

				SceneComp->SetupAttachment(AttachTarget, GeneratedComponent.ParentSocket);
				SceneComp->RegisterComponent();
				SceneComp->SetRelativeTransform(GeneratedComponent.RelativeOffset);
			}
			else
			{
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
