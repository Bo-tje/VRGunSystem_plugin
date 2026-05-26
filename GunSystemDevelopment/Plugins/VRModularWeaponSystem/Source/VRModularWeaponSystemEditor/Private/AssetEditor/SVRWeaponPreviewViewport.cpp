#include "AssetEditor/SVRWeaponPreviewViewport.h"
#include "AssetEditor/VRWeaponDataAssetEditor.h"
#include "Data/VRWeaponData.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "EditorViewportClient.h"

void SVRWeaponPreviewViewport::Construct(const FArguments& InArgs, TSharedPtr<FVRWeaponDataAssetEditor> InAssetEditor)
{
	AssetEditorPtr = InAssetEditor;

	// Create Preview Scene
	PreviewScene = MakeUnique<FAdvancedPreviewScene>(FPreviewScene::ConstructionValues(), -15.0f);
	PreviewScene->SetFloorVisibility(true);

	// Spawn Preview Actor
	UWorld* PreviewWorld = PreviewScene->GetWorld();
	if (PreviewWorld)
	{
		PreviewActor = PreviewWorld->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity);
		USceneComponent* RootComp = NewObject<USceneComponent>(PreviewActor, TEXT("PreviewRoot"));
		PreviewActor->SetRootComponent(RootComp);
		RootComp->RegisterComponent();
	}

	// Initialize Viewport Widget
	SEditorViewport::Construct(SEditorViewport::FArguments());

	RebuildWeaponPreview();
}

SVRWeaponPreviewViewport::~SVRWeaponPreviewViewport()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

TSharedRef<FEditorViewportClient> SVRWeaponPreviewViewport::MakeEditorViewportClient()
{
	ViewportClient = MakeShareable(new FVRWeaponViewportClient(*PreviewScene, SharedThis(this)));
	ViewportClient->ViewportType = LVT_Perspective;
	ViewportClient->bSetListenerPosition = false;
	return ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SVRWeaponPreviewViewport::MakeViewportToolbar()
{
	return SNullWidget::NullWidget;
}

void SVRWeaponPreviewViewport::RebuildWeaponPreview()
{
	if (!PreviewActor) return;

	TSharedPtr<FVRWeaponDataAssetEditor> AssetEditor = AssetEditorPtr.Pin();
	if (!AssetEditor.IsValid()) return;

	UVRWeaponData* WeaponData = AssetEditor->GetWeaponData();
	if (!WeaponData) return;

	// 1. Destroy old components
	TArray<UActorComponent*> ExistingComponents;
	PreviewActor->GetComponents(ExistingComponents);
	for (UActorComponent* Comp : ExistingComponents)
	{
		if (Comp && Comp != PreviewActor->GetRootComponent())
		{
			Comp->DestroyComponent();
		}
	}

	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, true);

	// 2. Build Base Weapon Parts
	for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
	{
		if (Part.PartName.IsNone() || Part.Mesh.IsNull()) continue;

		UStaticMesh* LoadedMesh = Part.Mesh.LoadSynchronous();
		if (!LoadedMesh) continue;

		UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(PreviewActor, Part.PartName);
		if (NewComponent)
		{
			NewComponent->SetStaticMesh(LoadedMesh);
			NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			USceneComponent* AttachTarget = PreviewActor->GetRootComponent();
			if (!Part.ParentSocket.IsNone())
			{
				TArray<UStaticMeshComponent*> MeshComps;
				PreviewActor->GetComponents(MeshComps);
				for (UStaticMeshComponent* MC : MeshComps)
				{
					if (MC && MC != NewComponent && MC->DoesSocketExist(Part.ParentSocket))
					{
						AttachTarget = MC;
						break;
					}
				}
			}

			NewComponent->AttachToComponent(AttachTarget, AttachRules, Part.ParentSocket);
			NewComponent->SetRelativeTransform(Part.PartOffset);
			NewComponent->RegisterComponent();
		}
	}

	// 3. Build Additional / Dynamic Components (Attachments)
	for (const FVRWeaponDynamicComponent& CompGen : WeaponData->AdditionalComponents)
	{
		if (CompGen.ComponentName.IsNone() || CompGen.OptionalMesh.IsNull()) continue;

		UStaticMesh* LoadedMesh = CompGen.OptionalMesh.LoadSynchronous();
		if (!LoadedMesh) continue;

		UStaticMeshComponent* NewComponent = NewObject<UStaticMeshComponent>(PreviewActor, CompGen.ComponentName);
		if (NewComponent)
		{
			NewComponent->SetStaticMesh(LoadedMesh);
			NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			USceneComponent* AttachTarget = PreviewActor->GetRootComponent();
			if (!CompGen.ParentSocket.IsNone())
			{
				TArray<UStaticMeshComponent*> MeshComps;
				PreviewActor->GetComponents(MeshComps);
				for (UStaticMeshComponent* MC : MeshComps)
				{
					if (MC && MC != NewComponent && MC->DoesSocketExist(CompGen.ParentSocket))
					{
						AttachTarget = MC;
						break;
					}
				}
			}

			NewComponent->AttachToComponent(AttachTarget, AttachRules, CompGen.ParentSocket);
			NewComponent->SetRelativeTransform(CompGen.RelativeOffset);
			NewComponent->RegisterComponent();
		}
	}

	// 4. Center camera on the rebuilt weapon bounds
	if (ViewportClient.IsValid())
	{
		FBox WeaponBounds = PreviewActor->GetComponentsBoundingBox(true);
		if (WeaponBounds.IsValid)
		{
			// Add a bit of padding to the bounds so it's not too tight
			WeaponBounds = WeaponBounds.ExpandBy(10.0f);
			ViewportClient->FocusViewportOnBox(WeaponBounds);
		}
		ViewportClient->Invalidate();
	}
}

FVRWeaponViewportClient::FVRWeaponViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SVRWeaponPreviewViewport>& InViewportWidget)
	: FEditorViewportClient(nullptr, &InPreviewScene, InViewportWidget)
{
	SetViewMode(VMI_Lit);
	bDrawAxes = true;
	EngineShowFlags.DisableAdvancedFeatures();
	EngineShowFlags.SetLighting(true);
	
	// Position default camera to look at the gun from side
	SetViewLocation(FVector(100.0f, -50.0f, 0.0f));
	SetViewRotation(FRotator(0.0f, 150.0f, 0.0f));
}
