#pragma once

#include "CoreMinimal.h"
#include "DemoTargets/VRTargetBase.h"
#include "VRTargetMoving.generated.h"

UCLASS(Blueprintable, BlueprintType)
class VRMODULARWEAPONSYSTEM_API AVRTargetMoving : public AVRTargetBase
{
	GENERATED_BODY()

public:
	AVRTargetMoving();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void KnockdownTarget() override;
	virtual void ResetTarget() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Movement")
	bool bIsMovementActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Movement")
	float MovementSpeed;

	/** If true, target reverses path when reaching the end. If false, loops back to the start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Movement")
	bool bPingPong;

	/** Simple relative offset to move towards if Waypoints is empty. relative to starting position. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Movement")
	FVector MovementOffset;

	/** List of relative points (relative to starting position) to move through. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Movement")
	TArray<FVector> Waypoints;

	/** If true, target stops moving when knocked down, and resumes moving upon resetting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target | Movement")
	bool bPauseMovementOnKnockdown;

private:
	FVector StartLocation;
	TArray<FVector> WorldWaypoints;
	int32 CurrentWaypointIndex;
	bool bMovingForward;
	bool bWasMovingBeforeKnockdown;
};
