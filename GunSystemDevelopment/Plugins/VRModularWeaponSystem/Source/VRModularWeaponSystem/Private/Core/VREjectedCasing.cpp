#include "Core/VREjectedCasing.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UnrealObjectPooler.h"

AVREjectedCasing::AVREjectedCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMeshComponent"));
	SetRootComponent(CasingMeshComponent);

	// Physics setup
	CasingMeshComponent->SetSimulatePhysics(true);
	CasingMeshComponent->SetNotifyRigidBodyCollision(true);
	CasingMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CasingMeshComponent->SetCollisionProfileName(TEXT("PhysicsBody"));
	CasingMeshComponent->SetGenerateOverlapEvents(false);
}

void AVREjectedCasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMeshComponent->OnComponentHit.AddDynamic(this, &AVREjectedCasing::OnCasingHit);
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(LifeSpanTimerHandle, this, &AVREjectedCasing::OnLifeSpanExpired, CasingLifeSpan, false);
	}
}

void AVREjectedCasing::InitializeCasing(UStaticMesh* Mesh, FVector ImpulseVelocity)
{
	if (Mesh && CasingMeshComponent)
	{
		CasingMeshComponent->SetStaticMesh(Mesh);
		
		// Reset physics state for recycled instances
		CasingMeshComponent->SetSimulatePhysics(false);
		CasingMeshComponent->SetSimulatePhysics(true);
		
		// Clear previous velocities
		CasingMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		CasingMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		
		// Reset sound cooldown time
		LastSoundTime = 0.0f;

		// Apply initial velocities
		CasingMeshComponent->SetPhysicsLinearVelocity(ImpulseVelocity);
		
		// Add some randomized spin for visual realism
		FVector RandomAngularVel(
			FMath::RandRange(-540.0f, 540.0f),
			FMath::RandRange(-540.0f, 540.0f),
			FMath::RandRange(-540.0f, 540.0f)
		);
		CasingMeshComponent->SetPhysicsAngularVelocityInDegrees(RandomAngularVel);

		// Schedule the lifespan timer
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(LifeSpanTimerHandle, this, &AVREjectedCasing::OnLifeSpanExpired, CasingLifeSpan, false);
		}
	}
}

void AVREjectedCasing::OnLifeSpanExpired()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
	}

	// Disable physics simulation to stop movement in the pool
	if (CasingMeshComponent)
	{
		CasingMeshComponent->SetSimulatePhysics(false);
		CasingMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		CasingMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	if (UUnrealObjectPooler* Pooler = GetWorld()->GetSubsystem<UUnrealObjectPooler>())
	{
		Pooler->ReturnObjectToPool(this);
	}
	else
	{
		Destroy();
	}
}

void AVREjectedCasing::OnCasingHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Prevent hitting our own weapon/hands and self-noise right at spawn
	if (OtherActor && OtherActor == GetOwner()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const float CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastSoundTime >= SoundCooldown)
	{
		float HitSpeed = CasingMeshComponent->GetPhysicsLinearVelocity().Size();
		if (HitSpeed > MinVelocityForSound && BounceSounds.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, BounceSounds.Num() - 1);
			if (USoundBase* SelectedSound = BounceSounds[RandomIndex])
			{
				UGameplayStatics::PlaySoundAtLocation(this, SelectedSound, Hit.Location);
			}
			LastSoundTime = CurrentTime;
		}
	}
}
