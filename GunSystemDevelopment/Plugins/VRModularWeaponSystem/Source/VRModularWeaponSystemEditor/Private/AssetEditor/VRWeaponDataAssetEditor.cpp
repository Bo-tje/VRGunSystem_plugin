#include "AssetEditor/VRWeaponDataAssetEditor.h"
#include "AssetEditor/SVRWeaponPreviewViewport.h"
#include "AssetEditor/SVRWeaponStatVisualizer.h"
#include "Data/VRWeaponData.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/Docking/SDockTab.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "VRWeaponDataAssetEditor"

const FName FVRWeaponDataAssetEditor::ViewportTabId(TEXT("VRWeaponDataAssetEditor_Viewport"));
const FName FVRWeaponDataAssetEditor::DetailsTabId(TEXT("VRWeaponDataAssetEditor_Details"));
const FName FVRWeaponDataAssetEditor::StatsTabId(TEXT("VRWeaponDataAssetEditor_Stats"));

FVRWeaponDataAssetEditor::FVRWeaponDataAssetEditor() {}
FVRWeaponDataAssetEditor::~FVRWeaponDataAssetEditor()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
	}
}

void FVRWeaponDataAssetEditor::InitVRWeaponDataAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UVRWeaponData* InWeaponData)
{
	WeaponData = InWeaponData;

	// Create Detail View
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NotifyHook = this;

	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(WeaponData);

	// Create the Layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_VRWeaponDataAssetEditor_Layout_v2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Horizontal)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.6f)
				->AddTab(ViewportTabId, ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.4f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.4f)
					->AddTab(StatsTabId, ETabState::OpenedTab)
				)
			)
		);

	const FName AppIdentifier = FName(TEXT("VRWeaponDataAssetEditorApp"));
	InitAssetEditor(Mode, InitToolkitHost, AppIdentifier, StandaloneDefaultLayout, true, true, InWeaponData);

	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
	}

	// Initial refresh to populate viewport and stats
	RefreshEditor();
}

FName FVRWeaponDataAssetEditor::GetToolkitFName() const 
{ 
	return FName("VRWeaponDataAssetEditor"); 
}

FText FVRWeaponDataAssetEditor::GetBaseToolkitName() const 
{ 
	return LOCTEXT("VRWeaponDataAssetEditorBaseName", "VR Weapon Data Editor"); 
}

FText FVRWeaponDataAssetEditor::GetToolkitName() const 
{ 
	return FText::FromString(WeaponData ? WeaponData->GetName() : TEXT("VR Weapon Data")); 
}

FString FVRWeaponDataAssetEditor::GetWorldCentricTabPrefix() const 
{ 
	return TEXT("VRWeaponData"); 
}

FLinearColor FVRWeaponDataAssetEditor::GetWorldCentricTabColorScale() const 
{ 
	return FLinearColor::White; 
}

void FVRWeaponDataAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("VRWeaponDataAssetEditorMenuCategory", "VR Weapon Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(ViewportTabId, FOnSpawnTab::CreateSP(this, &FVRWeaponDataAssetEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Live 3D Preview"))
		.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FVRWeaponDataAssetEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "Properties"))
		.SetGroup(WorkspaceMenuCategoryRef);

	InTabManager->RegisterTabSpawner(StatsTabId, FOnSpawnTab::CreateSP(this, &FVRWeaponDataAssetEditor::SpawnTab_Stats))
		.SetDisplayName(LOCTEXT("StatsTab", "Stat Dashboard"))
		.SetGroup(WorkspaceMenuCategoryRef);
}

void FVRWeaponDataAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(ViewportTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(StatsTabId);
}

TSharedRef<SDockTab> FVRWeaponDataAssetEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	ViewportWidget = SNew(SVRWeaponPreviewViewport, SharedThis(this));
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTabLabel", "Live 3D Preview"))
		[
			ViewportWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FVRWeaponDataAssetEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTabLabel", "Properties"))
		[
			DetailsView.ToSharedRef()
		];
}

TSharedRef<SDockTab> FVRWeaponDataAssetEditor::SpawnTab_Stats(const FSpawnTabArgs& Args)
{
	StatsWidget = SNew(SVRWeaponStatVisualizer, SharedThis(this));
	return SNew(SDockTab)
		.Label(LOCTEXT("StatsTabLabel", "Stat Dashboard"))
		[
			StatsWidget.ToSharedRef()
		];
}

void FVRWeaponDataAssetEditor::PostUndo(bool bSuccess)
{
	RefreshEditor();
}

void FVRWeaponDataAssetEditor::PostRedo(bool bSuccess)
{
	RefreshEditor();
}

void FVRWeaponDataAssetEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged)
{
	RefreshEditor();
}

void FVRWeaponDataAssetEditor::RefreshEditor()
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->RebuildWeaponPreview();
	}
	if (StatsWidget.IsValid())
	{
		StatsWidget->RefreshStats();
	}
}

#undef LOCTEXT_NAMESPACE
