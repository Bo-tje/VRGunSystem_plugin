#include "VRWeaponBase.h"
#include "VRWeaponData.h"
#include "VRWeaponComponentInterface.h"
#include "Components/StaticMeshComponent.h"

AVRWeaponBase::AVRWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot"));
	RootComponent = WeaponRoot;

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GripMesh"));
	FrameMesh->SetupAttachment(RootComponent);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(FrameMesh);

	MuzzleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MuzzleMesh"));
	MuzzleMesh->SetupAttachment(BarrelMesh);

	TriggerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TriggerMesh"));
	TriggerMesh->SetupAttachment(FrameMesh);

	SliderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SliderMesh"));
	SliderMesh->SetupAttachment(FrameMesh);
}

void AVRWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	ApplyWeaponDataVisuals();
	InitializeWeapon();
}

void AVRWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeWeapon();
}

void AVRWeaponBase::InitializeWeapon()
{
	if (!WeaponData) return;

	// Distribute data to all components that need it
	TArray<UActorComponent*> WeaponComponents;
	GetComponents(WeaponComponents);

	for (UActorComponent* Component : WeaponComponents)
	{
		if (Component && Component->Implements<UVRWeaponComponentInterface>())
		{
			IVRWeaponComponentInterface::Execute_InitializeComponent(Component, WeaponData);
		}
	}
}

void AVRWeaponBase::ApplyWeaponDataVisuals()
{
	if (!WeaponData) return;

	if (FrameMesh && WeaponData->FrameMesh) FrameMesh->SetStaticMesh(WeaponData->FrameMesh);
	if (BarrelMesh && WeaponData->BarrelMesh) BarrelMesh->SetStaticMesh(WeaponData->BarrelMesh);
	if (MuzzleMesh && WeaponData->MuzzleMesh) MuzzleMesh->SetStaticMesh(WeaponData->MuzzleMesh);
	if (TriggerMesh && WeaponData->TriggerMesh) TriggerMesh->SetStaticMesh(WeaponData->TriggerMesh);
	if (SliderMesh && WeaponData->SliderMesh) SliderMesh->SetStaticMesh(WeaponData->SliderMesh);
}

void AVRWeaponBase::PullTrigger_Implementation()
{
	// Delegate to all components that implement the interface (like FireComponent)
	TArray<UActorComponent*> TriggerComponents;
	GetComponents(TriggerComponents);

	for (UActorComponent* Component : TriggerComponents)
	{
		if (Component && Component->Implements<UVRWeaponInterface>())
		{
			IVRWeaponInterface::Execute_PullTrigger(Component);
		}
	}
}

void AVRWeaponBase::ReleaseTrigger_Implementation()
{
	TArray<UActorComponent*> TriggerComponents;
	GetComponents(TriggerComponents);

	for (UActorComponent* Component : TriggerComponents)
	{
		if (Component && Component->Implements<UVRWeaponInterface>())
		{
			IVRWeaponInterface::Execute_ReleaseTrigger(Component);
		}
	}
}
