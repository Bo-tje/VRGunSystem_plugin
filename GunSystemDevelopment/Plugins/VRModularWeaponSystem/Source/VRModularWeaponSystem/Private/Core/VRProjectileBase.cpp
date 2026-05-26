#include "Core/VRProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Data/ProjectileData.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

AVRProjectileBase::AVRProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(2.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName("BlockAllDynamic");
	CollisionComponent->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComponent->CanCharacterStepUpOn = ECB_No;
	RootComponent = CollisionComponent;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionProfileName("NoCollision");

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 40000.f;
	ProjectileMovement->MaxSpeed = 40000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
}

void AVRProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ProjectileMovement)
	{
		ProjectileMovement->OnProjectileStop.AddDynamic(this, &AVRProjectileBase::OnProjectileStop);
	}
}

void AVRProjectileBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (ProjectileData)
	{
		InitializeProjectile(ProjectileData);
	}
}

#include "UnrealObjectPooler.h"

AVRProjectileBase* AVRProjectileBase::SpawnProjectileFromData(const UObject* WorldContextObject, UProjectileData* InData, FTransform SpawnTransform, AActor* InOwner, APawn* InInstigator)
{
	if (!WorldContextObject || !InData || !InData->ProjectileClass) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	AActor* SpawnedActor = nullptr;

	// Spawn via UnrealObjectPooler subsystem if present, otherwise fallback to default spawn
	if (UUnrealObjectPooler* Pooler = World->GetSubsystem<UUnrealObjectPooler>())
	{
		SpawnedActor = Pooler->SpawnObject(InData->ProjectileClass, SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator(), EPoolType::GameObjects);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = InOwner;
		SpawnParams.Instigator = InInstigator;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnedActor = World->SpawnActor<AActor>(InData->ProjectileClass, SpawnTransform, SpawnParams);
	}

	if (AVRProjectileBase* VRProjectile = Cast<AVRProjectileBase>(SpawnedActor))
	{
		VRProjectile->SetOwner(InOwner);
		VRProjectile->SetInstigator(InInstigator);

		VRProjectile->InitializeProjectile(InData);
		return VRProjectile;
	}

	return nullptr;
}

void AVRProjectileBase::InitializeProjectile(UProjectileData* Data)
{
	ProjectileData = Data;

	if (ProjectileData)
	{
		if (ProjectileMovement)
		{
			ProjectileMovement->InitialSpeed = ProjectileData->InitialSpeed;
			ProjectileMovement->MaxSpeed = ProjectileData->InitialSpeed;
			ProjectileMovement->ProjectileGravityScale = ProjectileData->GravityScale;
			
			// Re-bind the updated component for pooled actors to ensure clean simulation state
			ProjectileMovement->SetUpdatedComponent(CollisionComponent);
			
			// Set the initial velocity in the forward direction
			ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileData->InitialSpeed;
			
			// Re-activate and reset the movement component for pooled actors
			ProjectileMovement->Activate(true);
		}

		if (ProjectileMesh && ProjectileData->LiveRoundMesh)
		{
			ProjectileMesh->SetStaticMesh(ProjectileData->LiveRoundMesh);
		}

		// Ensure actor is visible and collision is active (since pooler might have hidden/disabled it)
		SetActorHiddenInGame(false);
		if (CollisionComponent)
		{
			CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			// Prevent bullet from colliding with the weapon, its attachments, hands, or character
			if (GetOwner())
			{
				TArray<AActor*> IgnoredActors;
				
				// 1. Traverse up the attachment hierarchy (ignores weapon, hand meshes, player character)
				AActor* CurrentAncestor = GetOwner();
				while (CurrentAncestor)
				{
					IgnoredActors.Add(CurrentAncestor);
					CurrentAncestor = CurrentAncestor->GetAttachParentActor();
				}
				
				// 2. Add any attachments to the weapon (sights, magazines, silencers, etc.)
				TArray<AActor*> AttachedActors;
				GetOwner()->GetAttachedActors(AttachedActors, true);
				IgnoredActors.Append(AttachedActors);
				
				// 3. Add instigator if not already added
				if (GetInstigator())
				{
					IgnoredActors.AddUnique(GetInstigator());
				}

				// 4. Set ignore list systems to avoid immediate self-collision
				for (AActor* ActorToIgnore : IgnoredActors)
				{
					if (ActorToIgnore)
					{
						CollisionComponent->IgnoreActorWhenMoving(ActorToIgnore, true);
					}
				}
			}
		}
	}
}

void AVRProjectileBase::OnProjectileStop(const FHitResult& ImpactResult)
{
	if (ProjectileData)
	{
		// Deal damage
		if (AActor* HitActor = ImpactResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitActor, ProjectileData->Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		}

		// Play Effects
		if (ProjectileData->ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ProjectileData->ImpactSound, ImpactResult.ImpactPoint);
		}

		if (ProjectileData->ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ProjectileData->ImpactEffect, ImpactResult.ImpactPoint, ImpactResult.ImpactNormal.Rotation());
		}
	}

	// Deactivate movement before returning to pool
	if (ProjectileMovement)
	{
		ProjectileMovement->Deactivate();
	}

	if (UWorld* World = GetWorld())
	{
		// Return to pool if pooler is available
		if (UUnrealObjectPooler* Pooler = World->GetSubsystem<UUnrealObjectPooler>())
		{
			SetOwner(nullptr);
			SetInstigator(nullptr);
			Pooler->ReturnObjectToPool(this);
			return;
		}
	}

	Destroy();
}
