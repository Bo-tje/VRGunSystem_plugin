#include "DemoTargets/VRTargetBase.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"

AVRTargetBase::AVRTargetBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	TargetMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetMesh"));
	TargetMesh->SetupAttachment(RootComponent);
	TargetMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	TargetMesh->SetGenerateOverlapEvents(true);

	bKnockdownOnHit = true;
	KnockdownAngle = 90.0f;
	KnockdownSpeed = 8.0f;
	ResetDelay = 3.0f;
	CurrentState = EVRTargetState::Active;

	DefaultMeshRotation = FRotator::ZeroRotator;
	TargetMeshRotation = FRotator::ZeroRotator;
}

void AVRTargetBase::BeginPlay()
{
	Super::BeginPlay();

	if (TargetMesh)
	{
		DefaultMeshRotation = TargetMesh->GetRelativeRotation();
		TargetMeshRotation = DefaultMeshRotation;
	}
}

void AVRTargetBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TargetMesh)
	{
		FRotator CurrentRotation = TargetMesh->GetRelativeRotation();
		if (!CurrentRotation.Equals(TargetMeshRotation, 0.1f))
		{
			FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetMeshRotation, DeltaTime, KnockdownSpeed);
			TargetMesh->SetRelativeRotation(NewRotation);
		}
		else if (CurrentState == EVRTargetState::Resetting)
		{
			// Snap to exact default and restore active status
			TargetMesh->SetRelativeRotation(DefaultMeshRotation);
			CurrentState = EVRTargetState::Active;
			TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

float AVRTargetBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentState != EVRTargetState::Active)
	{
		return 0.f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// Extract hit info
	FVector HitLocation = GetActorLocation();
	FVector HitNormal = GetActorUpVector();

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent& PointDamageEvent = *static_cast<const FPointDamageEvent*>(&DamageEvent);
		HitLocation = PointDamageEvent.HitInfo.ImpactPoint;
		HitNormal = PointDamageEvent.HitInfo.ImpactNormal;
	}
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent& RadialDamageEvent = *static_cast<const FRadialDamageEvent*>(&DamageEvent);
		if (RadialDamageEvent.ComponentHits.Num() > 0)
		{
			HitLocation = RadialDamageEvent.ComponentHits[0].ImpactPoint;
			HitNormal = RadialDamageEvent.ComponentHits[0].ImpactNormal;
		}
	}

	// Play Hit Sound
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitLocation);
	}

	// Spawn Hit Niagara Particle Effect
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, HitLocation, HitNormal.Rotation());
	}

	// Broadcast Event
	OnTargetHit.Broadcast(ActualDamage, HitLocation, HitNormal, DamageCauser);

	// Perform knockdown if enabled
	if (bKnockdownOnHit)
	{
		KnockdownTarget();
	}

	return ActualDamage;
}

void AVRTargetBase::KnockdownTarget()
{
	if (CurrentState != EVRTargetState::Active) return;

	CurrentState = EVRTargetState::KnockedDown;
	TargetMeshRotation = DefaultMeshRotation + FRotator(-KnockdownAngle, 0.f, 0.f);

	if (TargetMesh)
	{
		TargetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnTargetKnockedDown.Broadcast();

	// Schedule Reset
	GetWorldTimerManager().SetTimer(ResetTimerHandle, this, &AVRTargetBase::ResetTarget, ResetDelay, false);
}

void AVRTargetBase::ResetTarget()
{
	if (CurrentState == EVRTargetState::Active) return;

	CurrentState = EVRTargetState::Resetting;
	TargetMeshRotation = DefaultMeshRotation;

	GetWorldTimerManager().ClearTimer(ResetTimerHandle);

	OnTargetReset.Broadcast();
}
