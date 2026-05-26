#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Misc/NotifyHook.h"
#include "EditorUndoClient.h"

class UVRWeaponData;
class IDetailsView;
class SVRWeaponPreviewViewport;
class SVRWeaponStatVisualizer;

class FVRWeaponDataAssetEditor : public FAssetEditorToolkit, public FNotifyHook, public FEditorUndoClient
{
public:
	FVRWeaponDataAssetEditor();
	virtual ~FVRWeaponDataAssetEditor() override;

	void InitVRWeaponDataAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UVRWeaponData* InWeaponData);

	// IToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	// FAssetEditorToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	// FEditorUndoClient interface
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

	// FNotifyHook interface
	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;

	UVRWeaponData* GetWeaponData() const { return WeaponData; }

	// Trigger live refresh of preview and stats
	void RefreshEditor();

private:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Stats(const FSpawnTabArgs& Args);

private:
	UVRWeaponData* WeaponData = nullptr;

	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<SVRWeaponPreviewViewport> ViewportWidget;
	TSharedPtr<SVRWeaponStatVisualizer> StatsWidget;

	static const FName ViewportTabId;
	static const FName DetailsTabId;
	static const FName StatsTabId;
};
