#include "Core/VRAttachmentActor.h"
#include "Components/StaticMeshComponent.h"
#include "Interaction/VRGrabComponent.h"
#include "Components/VRAttachmentPointComponent.h"

AVRAttachmentActor::AVRAttachmentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetSimulatePhysics(true);
}

void AVRAttachmentActor::BeginPlay()
{
	Super::BeginPlay();

	if (UVRGrabComponent* GrabComp = FindComponentByClass<UVRGrabComponent>())
	{
		GrabComp->OnGrabbed.AddDynamic(this, &AVRAttachmentActor::HandleGrabbed);
		GrabComp->OnGrabReleased.AddDynamic(this, &AVRAttachmentActor::HandleReleased);
	}
}

void AVRAttachmentActor::HandleGrabbed(AActor* InteractingHand)
{
	bIsHeld = true;
}

void AVRAttachmentActor::HandleReleased()
{
	bIsHeld = false;

	TArray<UPrimitiveComponent*> OverlappingComponents;
	GetOverlappingComponents(OverlappingComponents);

	for (UPrimitiveComponent* Comp : OverlappingComponents)
	{
		if (UVRAttachmentPointComponent* AttachPoint = Cast<UVRAttachmentPointComponent>(Comp))
		{
			if (AttachPoint->TryAttach(this))
			{
				break;
			}
		}
	}
}
