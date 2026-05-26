#include "DemoTargets/VRTargetMoving.h"

AVRTargetMoving::AVRTargetMoving()
{
	bIsMovementActive = true;
	MovementSpeed = 200.0f;
	bPingPong = true;
	MovementOffset = FVector(0.f, 300.f, 0.f);
	bPauseMovementOnKnockdown = true;

	CurrentWaypointIndex = 0;
	bMovingForward = true;
	bWasMovingBeforeKnockdown = true;
}

void AVRTargetMoving::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();

	// Initialize world waypoints
	WorldWaypoints.Empty();
	if (Waypoints.Num() > 0)
	{
		for (const FVector& RelativeWP : Waypoints)
		{
			WorldWaypoints.Add(StartLocation + RelativeWP);
		}
	}
	else
	{
		// Fallback to start location and movement offset
		WorldWaypoints.Add(StartLocation);
		WorldWaypoints.Add(StartLocation + MovementOffset);
	}

	if (WorldWaypoints.Num() > 1)
	{
		CurrentWaypointIndex = 1;
	}
	else
	{
		CurrentWaypointIndex = 0;
	}
}

void AVRTargetMoving::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only move if movement is active and we are in an active standing state
	const bool bCanMove = bIsMovementActive && (CurrentState == EVRTargetState::Active);

	if (bCanMove && WorldWaypoints.Num() > 1)
	{
		FVector CurrentLoc = GetActorLocation();
		FVector TargetLoc = WorldWaypoints[CurrentWaypointIndex];

		FVector MoveDirection = (TargetLoc - CurrentLoc).GetSafeNormal();
		float DistanceToTarget = FVector::Dist(CurrentLoc, TargetLoc);
		float MoveDist = MovementSpeed * DeltaTime;

		if (MoveDist >= DistanceToTarget)
		{
			SetActorLocation(TargetLoc);

			// Choose next waypoint
			if (bPingPong)
			{
				if (bMovingForward)
				{
					CurrentWaypointIndex++;
					if (CurrentWaypointIndex >= WorldWaypoints.Num())
					{
						CurrentWaypointIndex = WorldWaypoints.Num() - 2;
						if (CurrentWaypointIndex < 0) CurrentWaypointIndex = 0;
						bMovingForward = false;
					}
				}
				else
				{
					CurrentWaypointIndex--;
					if (CurrentWaypointIndex < 0)
					{
						CurrentWaypointIndex = 1;
						if (CurrentWaypointIndex >= WorldWaypoints.Num()) CurrentWaypointIndex = 0;
						bMovingForward = true;
					}
				}
			}
			else
			{
				CurrentWaypointIndex = (CurrentWaypointIndex + 1) % WorldWaypoints.Num();
			}
		}
		else
		{
			SetActorLocation(CurrentLoc + MoveDirection * MoveDist);
		}
	}
}

void AVRTargetMoving::KnockdownTarget()
{
	if (bPauseMovementOnKnockdown)
	{
		bWasMovingBeforeKnockdown = bIsMovementActive;
		bIsMovementActive = false;
	}

	Super::KnockdownTarget();
}

void AVRTargetMoving::ResetTarget()
{
	Super::ResetTarget();

	if (bPauseMovementOnKnockdown)
	{
		bIsMovementActive = bWasMovingBeforeKnockdown;
	}
}
