#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/VRInteractableInterface.h"
#include "Data/VRWeaponStats.h"
#include "VRAttachmentActor.generated.h"

UCLASS()
class VRMODULARWEAPONSYSTEM_API AVRAttachmentActor : public AActor, public IVRInteractableInterface
{
	GENERATED_BODY()

public:
	AVRAttachmentActor();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleGrabbed(AActor* InteractingHand);

	UFUNCTION()
	void HandleReleased();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Attachment")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Attachment")
	TObjectPtr<UVRWeaponStatModifier> StatModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Attachment")
	FName AttachmentSocketName;

	UPROPERTY(BlueprintReadOnly, Category = "VR Attachment")
	bool bIsHeld = false;

	// --- IVRInteractableInterface ---
	virtual void StartAction_Implementation(UObject* Interactor, float ActionValue, FGameplayTag ActionTag) override {}
	virtual void StopAction_Implementation(UObject* Interactor, FGameplayTag ActionTag) override {}
	virtual void OnHoverStart_Implementation(UObject* Interactor) override {}
	virtual void OnHoverEnd_Implementation(UObject* Interactor) override {}
};
