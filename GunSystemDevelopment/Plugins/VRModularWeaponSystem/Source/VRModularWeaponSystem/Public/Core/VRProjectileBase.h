#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRProjectileBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UProjectileData;
class APawn;

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVRProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AVRProjectileBase();

	virtual void PostInitializeComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Static helper function to spawn/retrieve a projectile from data, supporting object pooling */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Projectile", meta = (WorldContext = "WorldContextObject"))
	static AVRProjectileBase* SpawnProjectileFromData(const UObject* WorldContextObject, UProjectileData* InData, FTransform SpawnTransform, AActor* InOwner = nullptr, APawn* InInstigator = nullptr);

	/** Called to set up the projectile's stats and visuals based on the Data Asset */
	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Projectile")
	void InitializeProjectile(UProjectileData* Data);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UNiagaraComponent> TrailComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Projectile Data")
	TObjectPtr<UProjectileData> ProjectileData;

	UFUNCTION()
	virtual void OnProjectileStop(const FHitResult& ImpactResult);
};
