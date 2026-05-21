#include "Core/VRMagazineBase.h"
#include "Interaction/VRGrabComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/MagazineData.h"
#include "Data/ProjectileData.h"

AVRMagazineBase::AVRMagazineBase()
{
	PrimaryActorTick.bCanEverTick = false;

	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
	RootComponent = GrabComponent;
	GrabComponent->SetCollisionProfileName("PhysicsActor");
	GrabComponent->SetSimulatePhysics(true);

	MagazineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagazineMesh"));
	MagazineMesh->SetupAttachment(RootComponent);
	MagazineMesh->SetCollisionProfileName("NoCollision");
}

void AVRMagazineBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MagazineData && MagazineMesh)
	{
		if (MagazineData->MagazineMesh)
		{
			MagazineMesh->SetStaticMesh(MagazineData->MagazineMesh);
		}
	}
}

void AVRMagazineBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (MagazineData && MagazineMesh)
	{
		if (MagazineData->MagazineMesh)
		{
			MagazineMesh->SetStaticMesh(MagazineData->MagazineMesh);
		}
	}

	RefillMagazine();
}

void AVRMagazineBase::BeginPlay()
{
	Super::BeginPlay();

	RefillMagazine();
}

bool AVRMagazineBase::GetRound_Implementation(UProjectileData*& OutRound)
{
	if (CurrentAmmo > 0 && MagazineData && MagazineData->ProjectileData)
	{
		CurrentAmmo--;
		OutRound = MagazineData->ProjectileData;
		UpdateVisualBullets();

		if (CurrentAmmo <= 0)
		{
			OnAmmoEmpty.Broadcast();
		}

		return true;
	}

	OutRound = nullptr;
	return false;
}

void AVRMagazineBase::RefillMagazine()
{
	if (MagazineData)
	{
		CurrentAmmo = MagazineData->MaxAmmo;
	}
	else
	{
		// Default to something if no data asset is assigned so it isn't completely useless
		CurrentAmmo = FMath::Max(CurrentAmmo, 30); 
	}

	SetupVisualBullets();
	UpdateVisualBullets();
}

void AVRMagazineBase::SetupVisualBullets()
{
	if (!bShowVisualBullets || !MagazineData || !MagazineData->ProjectileData || !MagazineData->ProjectileData->LiveRoundMesh)
	{
		return;
	}

	// Clean up existing meshes
	for (const TObjectPtr<UStaticMeshComponent>& MeshComp : VisualBulletMeshes)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}
	VisualBulletMeshes.Empty();

	// Create new meshes up to the capacity
	int32 Capacity = MagazineData ? MagazineData->MaxAmmo : CurrentAmmo;

	for (int32 i = 0; i < Capacity; ++i)
	{
		UStaticMeshComponent* BulletMesh = NewObject<UStaticMeshComponent>(this);
		if (BulletMesh)
		{
			BulletMesh->SetupAttachment(MagazineMesh);
			BulletMesh->RegisterComponent();
			BulletMesh->SetStaticMesh(MagazineData->ProjectileData->LiveRoundMesh);
			BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			
			// Offset them slightly based on the index
			FVector Offset = FirstBulletOffset + (BulletVisualOffset * i);
			BulletMesh->SetRelativeLocation(Offset);
			
			VisualBulletMeshes.Add(BulletMesh);
		}
	}
}

void AVRMagazineBase::UpdateVisualBullets()
{
	if (!bShowVisualBullets)
	{
		return;
	}

	// Hide or show bullets based on current ammo
	for (int32 i = 0; i < VisualBulletMeshes.Num(); ++i)
	{
		if (VisualBulletMeshes[i])
		{
			// if index < CurrentAmmo, the bullet should be visible.
			VisualBulletMeshes[i]->SetVisibility(i < CurrentAmmo);
		}
	}
}
