#include "Interaction/VRInteractor.h"
#include "Interaction/VRGrabComponent.h"
#include "Interfaces/VRInteractableInterface.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"

UVRInteractor::UVRInteractor()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	DetectionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Detection Sphere"));
	DetectionSphereComponent->SetupAttachment(this);
	DetectionSphereComponent->SetSphereRadius(10.0f);
	
	
	DetectionSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
    
	
	DetectionSphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    
	
	DetectionSphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Overlap);

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

void UVRInteractor::IntendActionStart(float ActionValue, FGameplayTag ActionTag)
{
	if (ActiveGrabComponent)
	{
		IVRInteractableInterface::Execute_StartAction(ActiveGrabComponent, this, ActionValue, ActionTag);
	}
}

void UVRInteractor::IntendActionStop(FGameplayTag ActionTag)
{
	if (ActiveGrabComponent)
	{
		IVRInteractableInterface::Execute_StopAction(ActiveGrabComponent, this, ActionTag);
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

	if (!OtherActor->Implements<UVRInteractableInterface>()) return; 
	
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

void UVRInteractor::PlayHapticFeedback(UHapticFeedbackEffect_Base* HapticEffect, float Scale, bool bLoop)
{
	if (!HapticEffect) return;
	
	if (GetWorld()->GetTimeSeconds() - LastHapticTime < 0.05f) 
	{
		return;
	}

	if (APlayerController* PC = GetProvidingPlayerController())
	{
		PC->PlayHapticEffect(HapticEffect, HandSide, Scale, bLoop);
		LastHapticTime = GetWorld()->GetTimeSeconds();
	}
}


void UVRInteractor::UpdateBestHoverTarget()
{
	UVRGrabComponent* NewBest = GetBestGrabTarget(); 
    
	if (NewBest != HoverTarget)
	{
		if (HoverTarget.IsValid())
		{
			UObject* OldObj = HoverTarget.Get();
			if (OldObj->Implements<UVRInteractableInterface>())
			{
				IVRInteractableInterface::Execute_OnHoverEnd(OldObj, this);
			}
			
			if (AActor* Owner = HoverTarget->GetOwner())
			{
				if (Owner->Implements<UVRInteractableInterface>())
				{
					IVRInteractableInterface::Execute_OnHoverEnd(Owner, this);
				}
			}
		}

		if (NewBest)
		{
			if (NewBest->Implements<UVRInteractableInterface>())
			{
				IVRInteractableInterface::Execute_OnHoverStart(NewBest, this);
			}

			if (AActor* Owner = NewBest->GetOwner())
			{
				if (Owner->Implements<UVRInteractableInterface>())
				{
					IVRInteractableInterface::Execute_OnHoverStart(Owner, this);
				}
			}
			
			if (APlayerController* PC = GetProvidingPlayerController())
			{
				PC->PlayHapticEffect(HoverHapticEffect, HandSide, 0.5f);
			}
		}

		HoverTarget = NewBest;
	}
}
