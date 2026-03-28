#include "VRInteractor.h"
#include "VRGrabComponent.h"
#include "VRInteractableInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"

UVRInteractor::UVRInteractor()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	DetectionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Detection Sphere"));
	DetectionSphereComponent->SetupAttachment(this);
	DetectionSphereComponent->SetSphereRadius(10.0f);
	DetectionSphereComponent->SetCollisionProfileName(TEXT("Trigger"));	
}

void UVRInteractor::BeginPlay()
{
	Super::BeginPlay();

	DetectionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UVRInteractor::OnSphereOverlapBegin);
	DetectionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &UVRInteractor::OnSphereOverlapEnd);
}

void UVRInteractor::IntendGrab()
{
	if (ActiveGrabComponent) return;

	if (UVRGrabComponent* TargetGrabComponent = GetBestGrabTarget())
	{
		TargetGrabComponent->TryGrab(this);
		ActiveGrabComponent = TargetGrabComponent;
	}
}

void UVRInteractor::IntendRelease()
{
	if (!ActiveGrabComponent) return;
	
	ActiveGrabComponent->TryRelease();
	ActiveGrabComponent = nullptr;
}

void UVRInteractor::IntendActionStart()
{
	if (ActiveGrabComponent)
	{
		IVRInteractableInterface::Execute_StartAction(ActiveGrabComponent, this);
	}
}

void UVRInteractor::IntendActionStop()
{
	if (ActiveGrabComponent)
	{
		IVRInteractableInterface::Execute_StopAction(ActiveGrabComponent, this);
	}
}

void UVRInteractor::RequestRelease()
{
	ActiveGrabComponent = nullptr;
}

UVRGrabComponent* UVRInteractor::GetBestGrabTarget() const
{
	UVRGrabComponent* BestCandidate = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	FVector InteractorLocation = GetComponentLocation();

	for (const TWeakObjectPtr<UVRGrabComponent>& GrabPtr : OverlappingGrabs)
	{
		if (GrabPtr.IsValid())
		{
			UVRGrabComponent* CurrentGrab = GrabPtr.Get();
			float CurrentDistSq = FVector::DistSquared(InteractorLocation, CurrentGrab->GetComponentLocation());

			if (CurrentDistSq < ClosestDistanceSq)
			{
				ClosestDistanceSq = CurrentDistSq;
				BestCandidate = CurrentGrab;
			}
		}
	}

	return BestCandidate;
}

void UVRInteractor::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	TArray<UVRGrabComponent*> GrabComponents;
	OtherActor->GetComponents<UVRGrabComponent>(GrabComponents);

	if (GrabComponents.Num() > 0)
	{
		for (UVRGrabComponent* Grab : GrabComponents)
		{
			OverlappingGrabs.AddUnique(Grab);
		}
        
		if (!GetWorld()->GetTimerManager().IsTimerActive(HoverTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(HoverTimerHandle, this, &UVRInteractor::UpdateBestHoverTarget, 0.1f, true);
		}
	}
}

void UVRInteractor::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	
	if (DetectionSphereComponent && !DetectionSphereComponent->IsOverlappingActor(OtherActor))
	{
		TArray<UVRGrabComponent*> GrabComponents;
		OtherActor->GetComponents<UVRGrabComponent>(GrabComponents);

		for (UVRGrabComponent* Grab : GrabComponents)
		{
			OverlappingGrabs.RemoveSingleSwap(Grab);
		}
		
		UpdateBestHoverTarget();
	}
}

APlayerController* UVRInteractor::GetProvidingPlayerController() const
{
	if (APawn* MyPawn = Cast<APawn>(GetOwner()))
	{
		return Cast<APlayerController>(MyPawn->GetController());
	}
	return nullptr;
}

void UVRInteractor::UpdateBestHoverTarget()
{
	UVRGrabComponent* NewBest = GetBestGrabTarget(); 
    
	if (NewBest != HoverTarget)
	{
		if (HoverTarget.IsValid())
		{
			IVRInteractableInterface::Execute_OnHoverEnd(HoverTarget.Get(), this);
			IVRInteractableInterface::Execute_OnHoverEnd(HoverTarget->GetOwner(), this);
		}

		if (NewBest)
		{
			IVRInteractableInterface::Execute_OnHoverStart(NewBest, this);
			IVRInteractableInterface::Execute_OnHoverStart(NewBest->GetOwner(), this);
			
			if (APlayerController* PC = GetProvidingPlayerController())
			{
				PC->PlayHapticEffect(HoverHapticEffect, HandSide, 0.5f);
			}
		}

		HoverTarget = NewBest;
	}
}
