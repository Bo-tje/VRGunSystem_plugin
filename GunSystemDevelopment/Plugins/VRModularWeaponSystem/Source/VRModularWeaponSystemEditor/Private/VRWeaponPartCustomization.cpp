#include "VRWeaponPartCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Data/VRWeaponData.h"
#include "Engine/StaticMeshSocket.h"
#include "Engine/StaticMesh.h"

TSharedRef<IPropertyTypeCustomization> FVRWeaponPartCustomization::MakeInstance()
{
	return MakeShareable(new FVRWeaponPartCustomization());
}

void FVRWeaponPartCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];
}

void FVRWeaponPartCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	UpdateAvailableSockets(PropertyHandle);

	uint32 NumChildren;
	PropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex);

		if (ChildHandle->GetProperty() && ChildHandle->GetProperty()->GetFName() == FName("ParentSocket"))
		{
			ParentSocketHandle = ChildHandle;

			ChildBuilder.AddCustomRow(ChildHandle->GetPropertyDisplayName())
				.NameContent()
				[
					ChildHandle->CreatePropertyNameWidget()
				]
				.ValueContent()
				[
					SNew(SComboBox<TSharedPtr<FName>>)
					.OptionsSource(&AvailableSockets)
					.OnGenerateWidget(this, &FVRWeaponPartCustomization::MakeSocketComboWidget)
					.OnSelectionChanged(this, &FVRWeaponPartCustomization::OnSocketSelectionChanged)
					[
						SNew(STextBlock)
						.Text(this, &FVRWeaponPartCustomization::GetSelectedSocketText)
					]
				];
		}
		else
		{
			ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
	}
}

void FVRWeaponPartCustomization::UpdateAvailableSockets(TSharedRef<IPropertyHandle> PropertyHandle)
{
	AvailableSockets.Empty();
	AvailableSockets.Add(MakeShareable(new FName(NAME_None)));

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);

	for (UObject* Outer : OuterObjects)
	{
		if (UVRWeaponData* WeaponData = Cast<UVRWeaponData>(Outer))
		{
			for (const FVRWeaponPart& Part : WeaponData->WeaponParts)
			{
				if (!Part.Mesh.IsNull() && Part.Mesh.IsValid())
				{
					if (UStaticMesh* SM = Part.Mesh.Get())
					{
						TArray<FString> SocketNames;
						// UStaticMesh sockets are accessible via finding the sockets array
						// Since 5.0, you can query sockets from the UStaticMesh
						TArray<UStaticMeshSocket*> Sockets = SM->Sockets;
						for (UStaticMeshSocket* Socket : Sockets)
						{
							AvailableSockets.Add(MakeShareable(new FName(Socket->SocketName)));
						}
					}
				}
			}
		}
	}
}

TSharedRef<SWidget> FVRWeaponPartCustomization::MakeSocketComboWidget(TSharedPtr<FName> InItem)
{
	return SNew(STextBlock).Text(FText::FromName(*InItem));
}

void FVRWeaponPartCustomization::OnSocketSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	if (NewSelection.IsValid() && ParentSocketHandle.IsValid())
	{
		ParentSocketHandle->SetValue(*NewSelection);
	}
}

FText FVRWeaponPartCustomization::GetSelectedSocketText() const
{
	if (ParentSocketHandle.IsValid())
	{
		FName CurrentValue;
		if (ParentSocketHandle->GetValue(CurrentValue) == FPropertyAccess::Success)
		{
			return FText::FromName(CurrentValue);
		}
	}
	return FText::FromString(TEXT("None"));
}
