#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "VRAttachmentPointComponent.generated.h"

class AVRAttachmentActor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRMODULARWEAPONSYSTEM_API UVRAttachmentPointComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UVRAttachmentPointComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Attachment")
	FName SocketName;

	UFUNCTION(BlueprintCallable, Category = "VR Attachment")
	bool TryAttach(AVRAttachmentActor* AttachmentActor);

	UFUNCTION(BlueprintCallable, Category = "VR Attachment")
	void Detach();

	UPROPERTY(BlueprintReadOnly, Category = "VR Attachment")
	TObjectPtr<AVRAttachmentActor> CurrentAttachment = nullptr;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
