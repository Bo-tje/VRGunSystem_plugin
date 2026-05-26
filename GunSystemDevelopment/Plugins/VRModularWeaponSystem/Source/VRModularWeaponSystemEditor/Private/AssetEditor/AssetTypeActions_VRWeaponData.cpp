#include "AssetEditor/AssetTypeActions_VRWeaponData.h"
#include "Data/VRWeaponData.h"
#include "AssetEditor/VRWeaponDataAssetEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_VRWeaponData"

FText FAssetTypeActions_VRWeaponData::GetName() const
{
	return LOCTEXT("VRWeaponDataName", "VR Weapon Data");
}

UClass* FAssetTypeActions_VRWeaponData::GetSupportedClass() const
{
	return UVRWeaponData::StaticClass();
}

FColor FAssetTypeActions_VRWeaponData::GetTypeColor() const
{
	return FColor(30, 160, 100); // A sleek custom dark-mint color for weapon assets
}

uint32 FAssetTypeActions_VRWeaponData::GetCategories()
{
	return EAssetTypeCategories::Gameplay;
}

void FAssetTypeActions_VRWeaponData::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (UVRWeaponData* WeaponData = Cast<UVRWeaponData>(*ObjIt))
		{
			TSharedRef<FVRWeaponDataAssetEditor> Editor = MakeShareable(new FVRWeaponDataAssetEditor());
			Editor->InitVRWeaponDataAssetEditor(Mode, EditWithinLevelEditor, WeaponData);
		}
	}
}

#undef LOCTEXT_NAMESPACE
