#include "DemoTargets/VRTargetWithHealth.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

AVRTargetWithHealth::AVRTargetWithHealth()
{
	MaxHealth = 100.0f;
	CurrentHealth = 100.0f;
	bKnockdownOnlyOnDestroy = true;
	bSimulatePhysicsOnDestroy = true;
	bHideMeshOnDestroy = false;
	bShouldReset = true;
	bIsDestroyed = false;
}

void AVRTargetWithHealth::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (TargetMesh)
	{
		InitialMeshTransform = TargetMesh->GetRelativeTransform();
	}
}

float AVRTargetWithHealth::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed)
	{
		return 0.f;
	}

	// Disable base knockdown animation on normal hits if configured to only knockdown on destroy
	const bool bOriginalKnockdownOnHit = bKnockdownOnHit;
	if (bKnockdownOnlyOnDestroy)
	{
		bKnockdownOnHit = false;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	bKnockdownOnHit = bOriginalKnockdownOnHit;

	CurrentHealth = FMath::Max(0.f, CurrentHealth - ActualDamage);
	OnTargetHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		DestroyTarget();
	}

	return ActualDamage;
}

void AVRTargetWithHealth::DestroyTarget()
{
	if (bIsDestroyed) return;
	bIsDestroyed = true;

	FVector Location = GetActorLocation();

	// Play Destroy Sound
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, Location);
	}

	// Spawn Destroy Niagara Effect
	if (DestroyEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestroyEffect, Location, GetActorRotation());
	}

	OnTargetDestroyed.Broadcast();

	if (bSimulatePhysicsOnDestroy)
	{
		if (TargetMesh)
		{
			TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			TargetMesh->SetSimulatePhysics(true);
			TargetMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			TargetMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			// Apply a neat backward impulse from the impact direction
			TargetMesh->AddImpulse(GetActorForwardVector() * 300.f, NAME_None, true);
		}

		if (bShouldReset)
		{
			GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &AVRTargetWithHealth::ResetTarget, ResetDelay, false);
		}
	}
	else if (bHideMeshOnDestroy)
	{
		if (TargetMesh)
		{
			TargetMesh->SetVisibility(false);
			TargetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (bShouldReset)
		{
			GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &AVRTargetWithHealth::ResetTarget, ResetDelay, false);
		}
	}
	else
	{
		// Fallback to standard base class knockdown
		KnockdownTarget();
	}
}

void AVRTargetWithHealth::ResetTarget()
{
	CurrentHealth = MaxHealth;
	bIsDestroyed = false;

	if (TargetMesh)
	{
		if (bSimulatePhysicsOnDestroy)
		{
			TargetMesh->SetSimulatePhysics(false);
			TargetMesh->SetRelativeTransform(InitialMeshTransform);
		}
		else if (bHideMeshOnDestroy)
		{
			TargetMesh->SetVisibility(true);
		}

		TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	Super::ResetTarget();

	OnTargetHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}
