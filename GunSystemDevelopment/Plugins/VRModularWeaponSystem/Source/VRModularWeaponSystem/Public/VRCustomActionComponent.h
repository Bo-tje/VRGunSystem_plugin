#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VRGrabComponent.h"
#include "VRCustomActionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRMODULARWEAPONSYSTEM_API UVRCustomActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRCustomActionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Plugin | Interaction", meta = (Categories = "VRModularWeaponSystem.Interaction"))
	FGameplayTag FilterTag;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | Interaction")
	FOnInputActionValue OnActionStart;

	UPROPERTY(BlueprintAssignable, Category = "VR Plugin | Interaction")
	FOnInputAction OnActionStop;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleStartAction(UObject* Interactor, float Value, FGameplayTag ActionTag);

	UFUNCTION()
	void HandleStopAction(UObject* Interactor, FGameplayTag ActionTag);

	UPROPERTY()
	UVRGrabComponent* CachedGrabComp;
};
