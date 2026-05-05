#include "Core/VRWeaponBase.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interaction/VRGrabComponent.h"
#include "Core/VRNativeTags.h"
#include "Interfaces/VRRoundProvider.h"
#include "Components/VRWeaponStateTreeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/VRMechanicalComponent.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRoot"));
	SetRootComponent(WeaponRoot);
	WeaponRoot->SetBoxExtent(FVector(1.0f)); // Tiny core for physics welding
	WeaponRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponRoot->SetCollisionProfileName(TEXT("PhysicsBody"));
	WeaponRoot->SetSimulatePhysics(true);

	PartRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PartRoot"));
	PartRoot->SetupAttachment(WeaponRoot);

	StateTreeComponent = CreateDefaultSubobject<UVRWeaponStateTreeComponent>(TEXT("StateTreeComponent"));
}

AVRWeaponBase* AVRWeaponBase::SpawnWeaponFromData(const UObject* WorldContextObject, UVRWeaponData* InData, FTransform SpawnTransform, TSubclassOf<AVRWeaponBase> WeaponClass)
{
	if (!WorldContextObject || !InData || !WeaponClass) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	// Use Deferred Spawning so we can inject the DataAsset BEFORE the Construction Script / ApplyVisuals runs
	AVRWeaponBase* NewWeapon = World->SpawnActorDeferred<AVRWeaponBase>(WeaponClass, SpawnTransform);
	if (NewWeapon)
	{
		NewWeapon->WeaponData = InData;
		NewWeapon->FinishSpawning(SpawnTransform);
	}

	return NewWeapon;
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
	
	CachedGrabComponents.Empty();
	GetComponents(CachedGrabComponents);
	
	for (UVRGrabComponent* GrabComponent : CachedGrabComponents)
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
	
	if (StateTreeComponent)
	{
		StateTreeComponent->SetStateTree(WeaponData->StateTree);
	}
	
	FAttachmentTransformRules StaticAttachRules(EAttachmentRule::KeepRelative, true);

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

			// CRITICAL: Use AttachToComponent with welding (true) for static parts.
			// Using SetupAttachment without explicit welding causes physics fighting between overlapping components.
			NewComponent->SetRelativeTransform(Part.PartOffset);
			NewComponent->AttachToComponent(AttachTarget, StaticAttachRules, Part.ParentSocket);
			NewComponent->RegisterComponent();
			
			NewComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			NewComponent->SetCollisionProfileName(TEXT("PhysicsBody"));
		}
	}
	
	for (const FVRWeaponDynamicComponent& CompGen : WeaponData->AdditionalComponents)
	{
		if (!CompGen.ComponentClass) continue;

		UActorComponent* NewObj = NewObject<UActorComponent>(this, CompGen.ComponentClass, CompGen.ComponentName);
		if (NewObj)
		{
			NewObj->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			
			DynamicComponentsMap.Add(CompGen.ComponentName, NewObj);
			
			if (NewObj->Implements<UVRWeaponComponentInterface>())
			{
				if (CompGen.Settings)
				{
					IVRWeaponComponentInterface::Execute_InitializeComponentWithSettings(NewObj, WeaponData, CompGen.Settings);
				}
				else
				{
					IVRWeaponComponentInterface::Execute_InitializeComponent(NewObj, WeaponData);
				}
			}
			
			if (USceneComponent* SceneComp = Cast<USceneComponent>(NewObj))
			{
				USceneComponent* AttachTarget = WeaponRoot;
				FName TargetSocket = CompGen.ParentSocket;
				
				if (!CompGen.ParentSocket.IsNone())
				{
					bool bAttachedToDynamic = false;
					// First, try to find a dynamic component by this name
					if (UActorComponent* ParentComp = GetDynamicComponentByName(CompGen.ParentSocket))
					{
						if (ParentComp != NewObj)
						{
							if (USceneComponent* ParentScene = Cast<USceneComponent>(ParentComp))
							{
								AttachTarget = ParentScene;
								TargetSocket = NAME_None;
								bAttachedToDynamic = true;
							}
						}
					}
					
					if (!bAttachedToDynamic)
					{
						// Otherwise, look for a socket on existing meshes
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
				}

				// Welding should only be done if specifically requested for dynamic parts,
				// or if they are static decorations (UStaticMeshComponent).
				// Mechanical parts should NOT be welded if they move.
				bool bShouldWeld = CompGen.bWeldCollision;
				if (NewObj->IsA<UStaticMeshComponent>() && !NewObj->IsA<UVRMechanicalComponent>())
				{
					bShouldWeld = true;
				}

				FAttachmentTransformRules DynAttachRules(EAttachmentRule::KeepRelative, bShouldWeld);
				SceneComp->SetRelativeTransform(CompGen.RelativeOffset);
				SceneComp->AttachToComponent(AttachTarget, DynAttachRules, TargetSocket);
				SceneComp->RegisterComponent();
				
				if (UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(NewObj))
				{
					if (!CompGen.OptionalMesh.IsNull())
					{
						SMComp->SetStaticMesh(CompGen.OptionalMesh.LoadSynchronous());
					}

					if (bShouldWeld)
					{
						SMComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						SMComp->SetCollisionProfileName(TEXT("PhysicsBody"));
					}
					else
					{
						// Safe defaults for moving or non-physical dynamic parts
						SMComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
						SMComp->SetCollisionProfileName(TEXT("NoCollision"));
					}
				}
				else if (UVRMechanicalComponent* MechComp = Cast<UVRMechanicalComponent>(NewObj))
				{
					if (!CompGen.OptionalMesh.IsNull())
					{
						MechComp->ConstructVisuals(CompGen.OptionalMesh.LoadSynchronous(), CompGen.bWeldCollision);
					}
				}
			}
			else
			{
				NewObj->RegisterComponent();
			}
		}
	}
}

UActorComponent* AVRWeaponBase::GetDynamicComponentByName(FName ComponentName) const
{
	if (UActorComponent* const* Component = DynamicComponentsMap.Find(ComponentName))
	{
		return *Component;
	}
	return nullptr;
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
		if (UVRMechanicalComponent* VisualTrigger = Cast<UVRMechanicalComponent>(GetDynamicComponentByName("Trigger")))
		{
			VisualTrigger->SetNormalizedValue(ActionValue);
		}
	}
	else if (ActionTag.MatchesTag(VRNativeTags::PrimaryInput))
	{
		IVRWeaponInterface::Execute_PrimaryAction(this);
	}
	else if (ActionTag.MatchesTag(VRNativeTags::SecondaryInput))
	{
		IVRWeaponInterface::Execute_SecondaryAction(this);
	}
	if (ActionTag.MatchesTag(VRNativeTags::Trigger) && ActionValue > 0.8f)
	{
		IVRWeaponInterface::Execute_PullTrigger(this);
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
