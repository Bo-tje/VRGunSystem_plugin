#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/VRRoundProvider.h"
#include "Interfaces/VRInteractableInterface.h"
#include "VRMagazineBase.generated.h"

class UVRGrabComponent;
class UMagazineData;
class UProjectileData;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAmmoEmpty);

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVRMagazineBase : public AActor, public IVRRoundProvider, public IVRInteractableInterface
{
	GENERATED_BODY()
	
public:	
	AVRMagazineBase();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// IVRRoundProvider
	virtual bool GetRound_Implementation(UProjectileData*& OutRound) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UVRGrabComponent> GrabComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MagazineMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Config")
	TObjectPtr<UMagazineData> MagazineData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Config")
	bool bShowVisualBullets = true;

	/** Offset for the very first bullet in the stack, used to align the stack with the magazine mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Config")
	FVector FirstBulletOffset = FVector::ZeroVector;

	/** Optional spacing offset between visual bullets if bShowVisualBullets is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine Config")
	FVector BulletVisualOffset = FVector(0, 0, -1.0f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Magazine State")
	int32 CurrentAmmo = 0;

	UPROPERTY(BlueprintAssignable, Category = "Magazine Events")
	FOnAmmoEmpty OnAmmoEmpty;

	UFUNCTION(BlueprintCallable, Category = "Magazine Logic")
	void RefillMagazine();

	UFUNCTION(BlueprintCallable, Category = "Magazine Logic")
	void SetupVisualBullets();
	
	UFUNCTION(BlueprintCallable, Category = "Magazine Logic")
	void UpdateVisualBullets();

protected:
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> VisualBulletMeshes;
};
