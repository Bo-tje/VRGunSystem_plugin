#include "VRModularWeaponSystemEditor.h"
#include "VRWeaponPartCustomization.h"
#include "Components/VRMechanicalComponent.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "PropertyEditorModule.h"
#include "AssetToolsModule.h"
#include "AssetEditor/AssetTypeActions_VRWeaponData.h"

#define LOCTEXT_NAMESPACE "FVRModularWeaponSystemEditorModule"

void FVRModularWeaponSystemEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("VRWeaponPart", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVRWeaponPartCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("VRWeaponDynamicComponent", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVRWeaponPartCustomization::MakeInstance));

	// Register Asset Type Actions for UVRWeaponData
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<FAssetTypeActions_VRWeaponData> Actions = MakeShareable(new FAssetTypeActions_VRWeaponData());
	VRWeaponDataAssetActions = Actions;
	AssetTools.RegisterAssetTypeActions(Actions);
}

void FVRModularWeaponSystemEditorModule::ShutdownModule()
{
	if (GUnrealEd != nullptr)
	{
		for (FName ClassName : RegisteredComponentClassNames)
		{
			GUnrealEd->UnregisterComponentVisualizer(ClassName);
		}
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		if (VRWeaponDataAssetActions.IsValid())
		{
			AssetTools.UnregisterAssetTypeActions(VRWeaponDataAssetActions.ToSharedRef());
		}
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("VRWeaponPart");
		PropertyModule.UnregisterCustomPropertyTypeLayout("VRWeaponDynamicComponent");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVRModularWeaponSystemEditorModule, VRModularWeaponSystemEditor)
