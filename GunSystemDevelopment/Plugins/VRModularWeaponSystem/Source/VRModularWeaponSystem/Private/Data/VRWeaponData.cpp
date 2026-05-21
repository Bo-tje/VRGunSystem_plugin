// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/VRWeaponData.h"

#if WITH_EDITOR
EDataValidationResult UVRWeaponData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	return Result;
}
#endif

TArray<FName> UVRWeaponData::GetAvailableComponentNames() const
{
	TArray<FName> ComponentNames;
	ComponentNames.Add(NAME_None);

	for (const FVRWeaponDynamicComponent& Comp : AdditionalComponents)
	{
		if (!Comp.ComponentName.IsNone())
		{
			ComponentNames.AddUnique(Comp.ComponentName);
		}
	}

	return ComponentNames;
}

