#include "VRModularWeaponSystemEditor.h"
#include "VRMechanicalComponentVisualizer.h"
#include "VRWeaponPartCustomization.h"
#include "Components/VRMechanicalComponent.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FVRModularWeaponSystemEditorModule"

void FVRModularWeaponSystemEditorModule::StartupModule()
{
	/*
	// Removing visualizer registration per user request
	if (GUnrealEd != nullptr)
	{
		TSharedPtr<FComponentVisualizer> Visualizer = MakeShareable(new FVRMechanicalComponentVisualizer());
		
		if (Visualizer.IsValid())
		{
			GUnrealEd->RegisterComponentVisualizer(UVRMechanicalComponent::StaticClass()->GetFName(), Visualizer);
			Visualizer->OnRegister();
			RegisteredComponentClassNames.Add(UVRMechanicalComponent::StaticClass()->GetFName());
		}
	}
	*/

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("VRWeaponPart", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVRWeaponPartCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("VRWeaponDynamicComponent", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FVRWeaponPartCustomization::MakeInstance));
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

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("VRWeaponPart");
		PropertyModule.UnregisterCustomPropertyTypeLayout("VRWeaponDynamicComponent");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVRModularWeaponSystemEditorModule, VRModularWeaponSystemEditor)
