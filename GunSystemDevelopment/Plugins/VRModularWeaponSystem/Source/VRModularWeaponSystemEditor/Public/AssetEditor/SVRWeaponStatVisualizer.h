#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FVRWeaponDataAssetEditor;

class SVRWeaponStatVisualizer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVRWeaponStatVisualizer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FVRWeaponDataAssetEditor> InAssetEditor);

	void RefreshStats();

private:
	TSharedRef<SWidget> BuildStatRow(const FText& StatName, float BaseVal, float FinalVal, bool bLowerIsBetter, float MaxExpectedValue, const FText& UnitText);

private:
	TWeakPtr<FVRWeaponDataAssetEditor> AssetEditorPtr;
	TSharedPtr<SVerticalBox> StatsBox;
};
