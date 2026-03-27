// Fill out your copyright notice in the Description page of Project Settings.


#include "VRInteractor.h"
#include "VRGrabComponent.h"
#include "VRInteractableInterface.h"
#include "Components/SphereComponent.h"

// Sets default values for this component's properties
UVRInteractor::UVRInteractor()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	DetectionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Detection Sphere"));
	DetectionSphereComponent->SetupAttachment(this);
	DetectionSphereComponent->SetSphereRadius(10.0f);
	DetectionSphereComponent->SetCollisionProfileName(TEXT("Trigger"));	
}

// Called when the game starts
void UVRInteractor::BeginPlay()
{
	Super::BeginPlay();

	// ...
	DetectionSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UVRInteractor::OnSphereOverlapBegin);
	DetectionSphereComponent->OnComponentEndOverlap.AddDynamic(this, &UVRInteractor::OnSphereOverlapEnd);

	
}

void UVRInteractor::IntendGrab()
{
	if (ActiveGrabComponent) return;

	if (UVRGrabComponent* TargetGrabComponent = GetBestGrabTarget())
	{
		if (TargetGrabComponent->IsHeld()) return;
		
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

UVRGrabComponent* UVRInteractor::GetBestGrabTarget() const
{
	UVRGrabComponent* BestCandidate = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max(); // Start at infinity
	FVector InteractorLocation = GetComponentLocation();

	// Loop through our list from the Overlap Events
	for (const TWeakObjectPtr<UVRGrabComponent>& GrabPtr : OverlappingGrabs)
	{
		// Safety check for the Weak Pointer
		if (GrabPtr.IsValid())
		{
			UVRGrabComponent* CurrentGrab = GrabPtr.Get();
            
			// Calculate Squared Distance
			float CurrentDistSq = FVector::DistSquared(InteractorLocation, CurrentGrab->GetOwner()->GetActorLocation());

			// If this is closer than our current best, update it
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
	if (UVRGrabComponent* FoundGrab = OtherActor->FindComponentByClass<UVRGrabComponent>())
	{
		OverlappingGrabs.AddUnique(FoundGrab);
        
		// Start a low-frequency Timer (e.g., every 0.1s) ONLY when the list isn't empty
		if (!GetWorld()->GetTimerManager().IsTimerActive(HoverTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(HoverTimerHandle, this, &UVRInteractor::UpdateBestHoverTarget, 0.1f, true);
		}
	}
}

void UVRInteractor::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UVRGrabComponent* FoundGrab = OtherActor->FindComponentByClass<UVRGrabComponent>())
	{
		OverlappingGrabs.RemoveSingleSwap(FoundGrab);
		
		if (FoundGrab == HoverTarget)
		{
			UpdateBestHoverTarget();
		}
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
		// No "if implements" check needed! Execute handles the "null" check internally.
		if (HoverTarget.IsValid())
		{
			IVRInteractableInterface::Execute_OnHoverEnd(HoverTarget->GetOwner(), this);
		}

		if (NewBest)
		{
			IVRInteractableInterface::Execute_OnHoverStart(NewBest->GetOwner(), this);
		}

		HoverTarget = NewBest;
	}
}
