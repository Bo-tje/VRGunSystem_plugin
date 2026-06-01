#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameplayTagContainer.h"
#include "Interfaces/VRWeaponComponentInterface.h"
#include "Interfaces/VRRoundProvider.h"
#include "VRMagwellComponent.generated.h"

class AVRMagazineBase;
class UVRWeaponData;
class UVRWeaponComponentSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagazineAttached, AVRMagazineBase*, Magazine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagazineDetached, AVRMagazineBase*, Magazine);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRMODULARWEAPONSYSTEM_API UVRMagwellComponent : public USphereComponent, public IVRWeaponComponentInterface, public IVRRoundProvider
{
	GENERATED_BODY()

public:	
	UVRMagwellComponent();

	virtual void BeginPlay() override;

	// IVRWeaponComponentInterface
	virtual void InitializeComponent_Implementation(UVRWeaponData* InData) override;
	virtual void InitializeComponentWithSettings_Implementation(UVRWeaponData* InData, UVRWeaponComponentSettings* InSettings) override;

	// IVRRoundProvider
	virtual bool GetRound_Implementation(UProjectileData*& OutRound) override;
	virtual bool HasRound_Implementation() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Weapon | Magwell")
	TObjectPtr<AVRMagazineBase> AttachedMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell")
	FName MagazineSocketName = TEXT("Magwell");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell", meta = (Categories = "VRModularWeaponSystem.MagazineType"))
	FGameplayTag CompatibleMagazinesTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell")
	bool bEjectOnRelease = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell")
	float InsertRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell | Feedback")
	TObjectPtr<USoundBase> InsertSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell | Feedback")
	TObjectPtr<USoundBase> EjectSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell | Feedback")
	TObjectPtr<UHapticFeedbackEffect_Base> HoverHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell | Feedback")
	TObjectPtr<UHapticFeedbackEffect_Base> InsertHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Weapon | Magwell | Feedback")
	TObjectPtr<UHapticFeedbackEffect_Base> EjectHapticEffect;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Magwell")
	FOnMagazineAttached OnMagazineAttached;

	UPROPERTY(BlueprintAssignable, Category = "VR Weapon | Magwell")
	FOnMagazineDetached OnMagazineDetached;

	UFUNCTION(BlueprintCallable, Category = "VR Weapon | Magwell")
	void EjectMagazine();

protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnMagazineGrabbed(AActor* InteractingHand);
	
	UFUNCTION()
	void OnMagazineReleased();

	UPROPERTY()
	TObjectPtr<AVRMagazineBase> OverlappingMagazine;
};
