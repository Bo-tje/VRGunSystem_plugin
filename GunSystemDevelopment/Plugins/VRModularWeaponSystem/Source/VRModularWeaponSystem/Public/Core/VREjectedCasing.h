#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VREjectedCasing.generated.h"

class UStaticMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVREjectedCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	AVREjectedCasing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CasingMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Casing | Config")
	float CasingLifeSpan = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Casing | Audio")
	TArray<TObjectPtr<USoundBase>> BounceSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Casing | Audio")
	float MinVelocityForSound = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Casing | Audio")
	float SoundCooldown = 0.15f;

	UFUNCTION(BlueprintCallable, Category = "Casing")
	void InitializeCasing(UStaticMesh* Mesh, FVector ImpulseVelocity);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnCasingHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void OnLifeSpanExpired();

private:
	float LastSoundTime = 0.0f;
	FTimerHandle LifeSpanTimerHandle;
};
