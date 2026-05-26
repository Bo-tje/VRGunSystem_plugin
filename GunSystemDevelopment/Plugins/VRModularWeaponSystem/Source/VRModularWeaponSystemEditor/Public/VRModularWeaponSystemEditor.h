#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IAssetTypeActions.h"

class FVRModularWeaponSystemEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	TArray<FName> RegisteredComponentClassNames;
	TSharedPtr<IAssetTypeActions> VRWeaponDataAssetActions;
};
