#include "AssetEditor/SVRWeaponStatVisualizer.h"
#include "AssetEditor/VRWeaponDataAssetEditor.h"
#include "Data/VRWeaponData.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"

void SVRWeaponStatVisualizer::Construct(const FArguments& InArgs, TSharedPtr<FVRWeaponDataAssetEditor> InAssetEditor)
{
	AssetEditorPtr = InAssetEditor;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
		.Padding(FMargin(16.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 16.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("VRWeaponStatVisualizer", "Title", "WEAPON PERFORMANCE DASHBOARD"))
				.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
				.ColorAndOpacity(FLinearColor(0.2f, 0.7f, 0.9f)) // Ice Blue accent
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SAssignNew(StatsBox, SVerticalBox)
			]
		]
	];

	RefreshStats();
}

void SVRWeaponStatVisualizer::RefreshStats()
{
	if (!StatsBox.IsValid()) return;

	StatsBox->ClearChildren();

	TSharedPtr<FVRWeaponDataAssetEditor> AssetEditor = AssetEditorPtr.Pin();
	if (!AssetEditor.IsValid()) return;

	UVRWeaponData* WeaponData = AssetEditor->GetWeaponData();
	if (!WeaponData) return;

	// 1. Calculate base vs final stats
	FVRWeaponStats Base = WeaponData->BaseStats;
	FVRWeaponStats Final = Base;

	for (const FVRWeaponDynamicComponent& CompGen : WeaponData->AdditionalComponents)
	{
		if (CompGen.StatModifiers)
		{
			Final.FireRate += CompGen.StatModifiers->FireRateOffset;
			Final.RecoilMultiplier *= CompGen.StatModifiers->RecoilMultiplier;
			Final.DamageMultiplier *= CompGen.StatModifiers->DamageMultiplier;
			Final.BulletVelocityMultiplier *= CompGen.StatModifiers->BulletVelocityMultiplier;
			Final.ReloadSpeedMultiplier *= CompGen.StatModifiers->ReloadSpeedMultiplier;
			Final.SpreadMultiplier *= CompGen.StatModifiers->SpreadMultiplier;
			Final.PelletCountOffset += CompGen.StatModifiers->PelletCountOffset;
		}
	}

	// 2. Add individual rows dynamically
	StatsBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			BuildStatRow(
				NSLOCTEXT("VRWeaponStatVisualizer", "FireRateLabel", "Fire Rate"),
				Base.FireRate, Final.FireRate, false, 1200.0f, FText::FromString(TEXT(" RPM"))
			)
		];

	StatsBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			BuildStatRow(
				NSLOCTEXT("VRWeaponStatVisualizer", "DamageLabel", "Damage Multiplier"),
				Base.DamageMultiplier, Final.DamageMultiplier, false, 3.0f, FText::FromString(TEXT("x"))
			)
		];

	StatsBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			BuildStatRow(
				NSLOCTEXT("VRWeaponStatVisualizer", "VelocityLabel", "Bullet Velocity"),
				Base.BulletVelocityMultiplier, Final.BulletVelocityMultiplier, false, 3.0f, FText::FromString(TEXT("x"))
			)
		];

	StatsBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			BuildStatRow(
				NSLOCTEXT("VRWeaponStatVisualizer", "ReloadLabel", "Reload Speed"),
				Base.ReloadSpeedMultiplier, Final.ReloadSpeedMultiplier, false, 3.0f, FText::FromString(TEXT("x"))
			)
		];

	StatsBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			BuildStatRow(
				NSLOCTEXT("VRWeaponStatVisualizer", "RecoilLabel", "Recoil Multiplier"),
				Base.RecoilMultiplier, Final.RecoilMultiplier, true, 2.0f, FText::FromString(TEXT("x"))
			)
		];

	StatsBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 4.0f)
		[
			BuildStatRow(
				NSLOCTEXT("VRWeaponStatVisualizer", "SpreadLabel", "Spread Multiplier"),
				Base.SpreadMultiplier, Final.SpreadMultiplier, true, 2.0f, FText::FromString(TEXT("x"))
			)
		];
}

TSharedRef<SWidget> SVRWeaponStatVisualizer::BuildStatRow(const FText& StatName, float BaseVal, float FinalVal, bool bLowerIsBetter, float MaxExpectedValue, const FText& UnitText)
{
	float Difference = FinalVal - BaseVal;
	bool bIsBetter = bLowerIsBetter ? (Difference < 0.0f) : (Difference > 0.0f);
	bool bIsUnchanged = FMath::IsNearlyZero(Difference);

	FLinearColor DeltaColor = bIsUnchanged ? FLinearColor(0.5f, 0.5f, 0.5f) : (bIsBetter ? FLinearColor(0.1f, 0.8f, 0.4f) : FLinearColor(0.9f, 0.3f, 0.3f));
	FLinearColor FillColor = bIsUnchanged ? FLinearColor(0.15f, 0.4f, 0.6f) : (bIsBetter ? FLinearColor(0.1f, 0.7f, 0.35f) : FLinearColor(0.8f, 0.25f, 0.25f));
	
	FString CompareString = FString::Printf(TEXT("%.2f%s -> %.2f%s"), BaseVal, *UnitText.ToString(), FinalVal, *UnitText.ToString());
	if (UnitText.ToString() == TEXT(" RPM"))
	{
		CompareString = FString::Printf(TEXT("%.0f%s -> %.0f%s"), BaseVal, *UnitText.ToString(), FinalVal, *UnitText.ToString());
	}
	
	FString DeltaString = bIsUnchanged ? TEXT("") : FString::Printf(TEXT(" (%s%.2f)"), Difference > 0.0f ? TEXT("+") : TEXT(""), Difference);
	if (UnitText.ToString() == TEXT(" RPM") && !bIsUnchanged)
	{
		DeltaString = FString::Printf(TEXT(" (%s%.0f)"), Difference > 0.0f ? TEXT("+") : TEXT(""), Difference);
	}

	float ProgressPercent = FMath::Clamp(FinalVal / MaxExpectedValue, 0.0f, 1.0f);

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("Menu.Background"))
		.Padding(FMargin(12.0f, 8.0f))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(StatName)
					.Font(FCoreStyle::Get().GetFontStyle("NormalFont"))
					.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(16.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(CompareString))
					.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(DeltaString))
					.ColorAndOpacity(DeltaColor)
					.Font(FCoreStyle::Get().GetFontStyle("BoldFont"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SProgressBar)
				.Percent(ProgressPercent)
				.FillColorAndOpacity(FillColor)
			]
		];
}
