#include "Core/VRWeaponBase.h"
#include "Engine/AssetManager.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interaction/VRGrabComponent.h"
#include "Core/VRNativeTags.h"
#include "Interfaces/VRRoundProvider.h"
#include "Components/VRWeaponStateTreeComponent.h"
#include "Components/VRWeaponFeedbackComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/VRMechanicalComponent.h"
#include "Data/VRWeaponStats.h"
#include "Interaction/VRInteractor.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponRoot = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRoot"));
	SetRootComponent(WeaponRoot);
	WeaponRoot->SetBoxExtent(FVector(1.0f)); 
	WeaponRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponRoot->SetCollisionProfileName(TEXT("PhysicsBody"));
	WeaponRoot->SetSimulatePhysics(true);

	PartRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PartRoot"));
	PartRoot->SetupAttachment(WeaponRoot);

	StateTreeComponent = CreateDefaultSubobject<UVRWeaponStateTreeComponent>(TEXT("StateTreeComponent"));
	FeedbackComponent = CreateDefaultSubobject<UVRWeaponFeedbackComponent>(TEXT("FeedbackComponent"));
}

AVRWeaponBase* AVRWeaponBase::SpawnWeaponFromData(const UObject* WorldContextObject, UVRWeaponData* InData, FTransform SpawnTransform, TSubclassOf<AVRWeaponBase> WeaponClass)
{
	if (!WorldContextObject || !InData || !WeaponClass) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

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
	
	// Ensure we don't duplicate components when construction script re-runs
	ClearDynamicComponents();
	
	if (WeaponData)
	{
		ApplyWeaponDataVisuals();
		InitializeWeapon();
	}
}

void AVRWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// Packaged builds do not serialize components created dynamically in the Construction Script.
	// We MUST rebuild them at runtime. To prevent physics explosions, we temporarily disable physics.
	bool bWasSimulating = false;
	if (WeaponRoot)
	{
		bWasSimulating = WeaponRoot->IsSimulatingPhysics();
		WeaponRoot->SetSimulatePhysics(false);
	}

	ClearDynamicComponents();
	ApplyWeaponDataVisuals();

	if (WeaponRoot && bWasSimulating)
	{
		WeaponRoot->SetSimulatePhysics(true);
	}
	
	InitializeWeapon();
	
	CachedGrabComponents.Empty();
	GetComponents(CachedGrabComponents);
	
	for (UVRGrabComponent* GrabComponent : CachedGrabComponents)
	{
		// Only bind to valid components, as DestroyComponent leaves old ones around until GC
		if (IsValid(GrabComponent))
		{
			GrabComponent->OnGrabbed.AddDynamic(this, &AVRWeaponBase::OnGrabbed);
			GrabComponent->OnGrabReleased.AddDynamic(this, &AVRWeaponBase::OnReleased);
		}
	}

	if (StateTreeComponent)
	{
		StateTreeComponent->StopLogic(TEXT("Init"));
	}
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

	UpdateCalculatedStats();
}

void AVRWeaponBase::UpdateCalculatedStats()
{
	if (!WeaponData) return;

	CalculatedStats = WeaponData->BaseStats;

	for (UActorComponent* Component : CachedWeaponComponents)
	{
		if (UVRWeaponStatModifier* Modifier = IVRWeaponComponentInterface::Execute_GetStatModifier(Component))
		{
			CalculatedStats.FireRate += Modifier->FireRateOffset;
			CalculatedStats.RecoilMultiplier *= Modifier->RecoilMultiplier;
			CalculatedStats.DamageMultiplier *= Modifier->DamageMultiplier;
			CalculatedStats.BulletVelocityMultiplier *= Modifier->BulletVelocityMultiplier;
			CalculatedStats.ReloadSpeedMultiplier *= Modifier->ReloadSpeedMultiplier;

			if (Modifier->MuzzleFlashOverride) CalculatedStats.MuzzleFlashOverride = Modifier->MuzzleFlashOverride;
			if (Modifier->FireSoundOverride) CalculatedStats.FireSoundOverride = Modifier->FireSoundOverride;
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
	
	TArray<FSoftObjectPath> AssetsToLoad;
	bool bAllLoaded = true;

	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (!Part.PartName.IsNone() && !Part.Mesh.IsNull())
		{
			if (Part.Mesh.IsPending())
			{
				AssetsToLoad.AddUnique(Part.Mesh.ToSoftObjectPath());
				bAllLoaded = false;
			}
		}
	}
	for (const FVRWeaponDynamicComponent& CompGen : WeaponData->AdditionalComponents)
	{
		if (!CompGen.OptionalMesh.IsNull())
		{
			if (CompGen.OptionalMesh.IsPending())
			{
				AssetsToLoad.AddUnique(CompGen.OptionalMesh.ToSoftObjectPath());
				bAllLoaded = false;
			}
		}
	}

	// Always use synchronous loading here to ensure the meshes are ready immediately.
	// This prevents race conditions where InitializeWeapon() runs before components exist.
	if (AssetsToLoad.Num() > 0)
	{
		for (const FSoftObjectPath& Path : AssetsToLoad)
		{
			UAssetManager::GetStreamableManager().LoadSynchronous(Path);
		}
	}
	
	ApplyWeaponDataVisuals_Internal();
}

void AVRWeaponBase::ApplyWeaponDataVisuals_Internal()
{
	if (!WeaponData) return;

	// Determine creation method: if we haven't started playing, we are likely in construction
	EComponentCreationMethod CreationMethod = HasActorBegunPlay() ? EComponentCreationMethod::Instance : EComponentCreationMethod::UserConstructionScript;

	FAttachmentTransformRules StaticAttachRules(EAttachmentRule::KeepRelative, true);

	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (Part.PartName.IsNone() || Part.Mesh.IsNull()) continue;
		
		UStaticMesh* LoadedMesh = Part.Mesh.Get();
		if (!LoadedMesh) continue;

		UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(this, Part.PartName);
		if (NewComponent)
		{
			NewComponent->CreationMethod = CreationMethod;
			if (CreationMethod == EComponentCreationMethod::Instance)
			{
				AddInstanceComponent(NewComponent);
			}
			NewComponent->SetStaticMesh(LoadedMesh);
			
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
			NewObj->CreationMethod = CreationMethod;
			if (CreationMethod == EComponentCreationMethod::Instance)
			{
				AddInstanceComponent(NewObj);
			}
			DynamicComponentsMap.Add(CompGen.ComponentName, NewObj);
			
			if (NewObj->Implements<UVRWeaponComponentInterface>())
			{
				if (CompGen.Settings)
				{
					// Register input tags for automatic routing
					for (const FGameplayTag& Tag : CompGen.Settings->BindToInputTags)
					{
						TagToComponentMap.Add(Tag, NewObj);
					}
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

				bool bShouldWeld = CompGen.bWeldCollision;
				if (NewObj->IsA<UStaticMeshComponent>() && !NewObj->IsA<UVRMechanicalComponent>()) bShouldWeld = true;

				FAttachmentTransformRules DynAttachRules(EAttachmentRule::KeepRelative, bShouldWeld);
				SceneComp->SetRelativeTransform(CompGen.RelativeOffset);
				SceneComp->AttachToComponent(AttachTarget, DynAttachRules, TargetSocket);
				SceneComp->RegisterComponent();
				
				if (UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(NewObj))
				{
					if (!CompGen.OptionalMesh.IsNull()) SMComp->SetStaticMesh(CompGen.OptionalMesh.Get());
					if (bShouldWeld)
					{
						SMComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
						SMComp->SetCollisionProfileName(TEXT("PhysicsBody"));
					}
					else
					{
						SMComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
						SMComp->SetCollisionProfileName(TEXT("NoCollision"));
					}
				}
				else if (UVRMechanicalComponent* MechComp = Cast<UVRMechanicalComponent>(NewObj))
				{
					if (!CompGen.OptionalMesh.IsNull()) MechComp->ConstructVisuals(CompGen.OptionalMesh.Get(), CompGen.bWeldCollision, CreationMethod);
				}
			}
			else
			{
				NewObj->RegisterComponent();
			}
		}
	}
}

void AVRWeaponBase::ClearDynamicComponents()
{
	// 1. Destroy components tracked in the map
	for (auto& Elem : DynamicComponentsMap)
	{
		if (UActorComponent* Comp = Elem.Value)
		{
			Comp->DestroyComponent();
		}
	}
	DynamicComponentsMap.Empty();

	// 2. Identify and destroy any static mesh components created by the part system
	TArray<UStaticMeshComponent*> AllSMComps;
	GetComponents(AllSMComps);

	for (UStaticMeshComponent* SM : AllSMComps)
	{
		if (SM == PartRoot || SM->IsDefaultSubobject()) continue;
		
		if (SM->CreationMethod == EComponentCreationMethod::UserConstructionScript || SM->CreationMethod == EComponentCreationMethod::Instance)
		{
			SM->DestroyComponent();
		}
	}
	
	TagToComponentMap.Empty();
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

		// Only start logic if this is the first interactor grabbing the weapon
		if (GetHoldingInteractors().Num() <= 1)
		{
			StateTreeComponent->StartLogic();
		}
	}
}

void AVRWeaponBase::OnReleased()
{
	// If we are still being held by another interactor (e.g. still holding main grip while releasing slider),
	// don't stop the weapon logic.
	if (GetHoldingInteractors().Num() > 0)
	{
		return;
	}

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
	// 1. Flexible Tag Routing (Manual Override in WeaponData)
	if (WeaponData && WeaponData->InputTagToComponentName.Contains(ActionTag))
	{
		const FName TargetName = WeaponData->InputTagToComponentName[ActionTag];
		if (UActorComponent* TargetComp = GetDynamicComponentByName(TargetName))
		{
			if (UVRMechanicalComponent* MechComp = Cast<UVRMechanicalComponent>(TargetComp))
			{
				MechComp->SetNormalizedValue(ActionValue);
			}
		}
	}
	// 1b. Automatic Tag Routing (from Component Settings)
	else if (UActorComponent** TargetCompPtr = TagToComponentMap.Find(ActionTag))
	{
		if (UVRMechanicalComponent* MechComp = Cast<UVRMechanicalComponent>(*TargetCompPtr))
		{
			MechComp->SetNormalizedValue(ActionValue);
		}
	}

	// 2. Event-Driven Logic
	if (ActionTag.MatchesTag(VRNativeTags::PrimaryInput))
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
	UVRMechanicalComponent* TargetMech = nullptr;

	// 1. Check Manual Override
	if (WeaponData && WeaponData->InputTagToComponentName.Contains(ActionTag))
	{
		const FName TargetName = WeaponData->InputTagToComponentName[ActionTag];
		TargetMech = Cast<UVRMechanicalComponent>(GetDynamicComponentByName(TargetName));
	}
	// 2. Check Automatic Mapping
	else if (UActorComponent** TargetCompPtr = TagToComponentMap.Find(ActionTag))
	{
		TargetMech = Cast<UVRMechanicalComponent>(*TargetCompPtr);
	}

	if (TargetMech)
	{
		if (!TargetMech->bHasReturnSpring) TargetMech->SetNormalizedValue(0.0f);
	}

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
	if (StateTreeComponent) StateTreeComponent->SendStateTreeEvent(VRNativeTags::Trigger);
}

void AVRWeaponBase::ReleaseTrigger_Implementation()
{
	if (!bIsTriggerPulled) return;
	bIsTriggerPulled = false;
	if (StateTreeComponent) StateTreeComponent->SendStateTreeEvent(VRNativeTags::TriggerReleased);
}

void AVRWeaponBase::PrimaryAction_Implementation()
{
	if (WeaponData && WeaponData->FireModes.Num() > 0)
	{
		CurrentFireModeIndex = (CurrentFireModeIndex + 1) % WeaponData->FireModes.Num();
	}
	if (StateTreeComponent) StateTreeComponent->SendStateTreeEvent(VRNativeTags::PrimaryInput);
}

void AVRWeaponBase::ReleasePrimaryAction_Implementation()
{
	if (StateTreeComponent) StateTreeComponent->SendStateTreeEvent(VRNativeTags::PrimaryInputReleased);
}

void AVRWeaponBase::SecondaryAction_Implementation()
{
	if (StateTreeComponent) StateTreeComponent->SendStateTreeEvent(VRNativeTags::SecondaryInput);
}

void AVRWeaponBase::ReleaseSecondaryAction_Implementation()
{
	if (StateTreeComponent) StateTreeComponent->SendStateTreeEvent(VRNativeTags::SecondaryInputReleased);
}

bool AVRWeaponBase::IsTriggerPulled_Implementation() const
{
	return bIsTriggerPulled;
}

void AVRWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Procedural Recoil Spring Logic
	if (WeaponData)
	{
		float Stiffness = CalculatedStats.RecoilSpringStiffness;
		float Damping = CalculatedStats.RecoilSpringDamping;

		// Simple damped spring force
		FRotator Force = (TargetRecoilOffset - CurrentRecoilOffset) * Stiffness - (RecoilVelocity * Damping);
		RecoilVelocity += Force * DeltaTime;
		CurrentRecoilOffset += RecoilVelocity * DeltaTime;

		// Decay target offset back to zero over time to reset
		TargetRecoilOffset = FMath::RInterpTo(TargetRecoilOffset, FRotator::ZeroRotator, DeltaTime, 10.0f);
	}

	// 2. Two-Handed "Virtual Stock" Stabilization
	TArray<UVRInteractor*> ActiveHolders = GetHoldingInteractors();
	FRotator FinalRotationOffset = CurrentRecoilOffset;
	FVector FinalLocationOffset = FVector::ZeroVector;

	if (ActiveHolders.Num() == 2 && ActiveHolders[0] && ActiveHolders[1])
	{
		// Basic implementation: Orient the weapon along the line between the two hands
		FVector PrimaryLocation = ActiveHolders[0]->GetOwner() ? ActiveHolders[0]->GetOwner()->GetActorLocation() : ActiveHolders[0]->GetComponentLocation();
		FVector SecondaryLocation = ActiveHolders[1]->GetOwner() ? ActiveHolders[1]->GetOwner()->GetActorLocation() : ActiveHolders[1]->GetComponentLocation();

		// Example pseudo-stock math (in a real game you'd blend this with the primary hand's rotation)
		FVector HandDir = (SecondaryLocation - PrimaryLocation).GetSafeNormal();
		if (!HandDir.IsNearlyZero())
		{
			// Add some smoothing or direct overriding here based on preference
			// For now, we scale down the recoil if held with two hands
			FinalRotationOffset *= 0.5f;
		}
	}

	// Apply offsets to PartRoot (or WeaponRoot)
	if (PartRoot)
	{
		PartRoot->SetRelativeRotation(FinalRotationOffset);
		PartRoot->SetRelativeLocation(FinalLocationOffset);
	}
}

FVRFireMode AVRWeaponBase::GetCurrentFireMode() const
{
	if (WeaponData && WeaponData->FireModes.IsValidIndex(CurrentFireModeIndex))
	{
		return WeaponData->FireModes[CurrentFireModeIndex];
	}
	return FVRFireMode();
}
