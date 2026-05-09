// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/VRWeaponData.h"

#if WITH_EDITOR
EDataValidationResult UVRWeaponData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
	return Result;
}
#endif
