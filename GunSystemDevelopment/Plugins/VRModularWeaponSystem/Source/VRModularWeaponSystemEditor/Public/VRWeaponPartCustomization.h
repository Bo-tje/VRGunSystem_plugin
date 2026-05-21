#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

class FVRWeaponPartCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, class IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TSharedPtr<IPropertyHandle> ParentSocketHandle;
	TArray<TSharedPtr<FName>> AvailableSockets;

	void UpdateAvailableSockets(TSharedRef<IPropertyHandle> PropertyHandle);
	TSharedRef<SWidget> MakeSocketComboWidget(TSharedPtr<FName> InItem);
	void OnSocketSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo);
	FText GetSelectedSocketText() const;
};
