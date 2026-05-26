#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"

class FVRWeaponDataAssetEditor;

class SVRWeaponPreviewViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SVRWeaponPreviewViewport) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVRWeaponDataAssetEditor> InAssetEditor);
	virtual ~SVRWeaponPreviewViewport() override;

	// SEditorViewport interface
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;

	void RebuildWeaponPreview();

private:
	TWeakPtr<FVRWeaponDataAssetEditor> AssetEditorPtr;
	TUniquePtr<FAdvancedPreviewScene> PreviewScene;
	TSharedPtr<class FVRWeaponViewportClient> ViewportClient;

	AActor* PreviewActor = nullptr;
};

class FVRWeaponViewportClient : public FEditorViewportClient
{
public:
	FVRWeaponViewportClient(FAdvancedPreviewScene& InPreviewScene, const TSharedRef<SVRWeaponPreviewViewport>& InViewportWidget);
};
