#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/VRInteractableInterface.h"
#include "VRRoundActor.generated.h"

class UVRGrabComponent;
class UProjectileData;
class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVRRoundActor : public AActor, public IVRInteractableInterface
{
	GENERATED_BODY()

public:
	AVRRoundActor();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVRGrabComponent> GrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RoundMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Round Config")
	TObjectPtr<UProjectileData> ProjectileData;

protected:
	void SetupVisualRound();
};
