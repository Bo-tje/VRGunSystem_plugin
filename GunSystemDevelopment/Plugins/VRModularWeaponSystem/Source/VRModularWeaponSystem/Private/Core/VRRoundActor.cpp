#include "Core/VRRoundActor.h"
#include "Interaction/VRGrabComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/ProjectileData.h"

AVRRoundActor::AVRRoundActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GrabComponent = CreateDefaultSubobject<UVRGrabComponent>(TEXT("GrabComponent"));
	RootComponent = GrabComponent;
	GrabComponent->SetCollisionProfileName("PhysicsActor");
	GrabComponent->SetSimulatePhysics(true);
	GrabComponent->bAttachOwnerOnGrab = true;

	RoundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoundMesh"));
	RoundMesh->SetupAttachment(RootComponent);
	RoundMesh->SetCollisionProfileName("NoCollision");
}

void AVRRoundActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetupVisualRound();
}

void AVRRoundActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetupVisualRound();
}

void AVRRoundActor::BeginPlay()
{
	Super::BeginPlay();
	SetupVisualRound();
}

void AVRRoundActor::SetupVisualRound()
{
	if (ProjectileData && RoundMesh)
	{
		if (ProjectileData->LiveRoundMesh)
		{
			RoundMesh->SetStaticMesh(ProjectileData->LiveRoundMesh);
		}
	}
}
