#include "VRCustomActionComponent.h"

UVRCustomActionComponent::UVRCustomActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVRCustomActionComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedGrabComp = GetOwner()->FindComponentByClass<UVRGrabComponent>();
	if (CachedGrabComp)
	{
		CachedGrabComp->StartAction.AddDynamic(this, &UVRCustomActionComponent::HandleStartAction);
		CachedGrabComp->StopAction.AddDynamic(this, &UVRCustomActionComponent::HandleStopAction);
	}
}

void UVRCustomActionComponent::HandleStartAction(UObject* Interactor, float Value, FGameplayTag ActionTag)
{
	if (ActionTag.MatchesTagExact(FilterTag))
	{
		OnActionStart.Broadcast(Interactor, Value);
	}
}

void UVRCustomActionComponent::HandleStopAction(UObject* Interactor, FGameplayTag ActionTag)
{
	if (ActionTag.MatchesTagExact(FilterTag))
	{
		OnActionStop.Broadcast(Interactor);
	}
}
