#include "StateTree/StateTreeWeaponEvaluator.h"
#include "StateTreeExecutionContext.h"
#include "Components/VRChamberComponent.h"
#include "Components/VRMagwellComponent.h"
#include "Components/VRInternalMagazineComponent.h"
#include "Core/VRMagazineBase.h"
#include "Interfaces/VRRoundProvider.h"
#include "Data/VRWeaponData.h"
#include "Core/VRWeaponBase.h"
#include "Components/VRWeaponStateTreeComponent.h"

void FSTEval_Weapon::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FStateTreeEvaluatorBase::Tick(Context, DeltaTime);
	
	FSTEval_WeaponInstanceData& InstanceData = Context.GetInstanceData<FSTEval_WeaponInstanceData>(*this);

	if (InstanceData.WeaponActor)
	{
		if (InstanceData.WeaponActor->Implements<UVRWeaponInterface>())
		{
			InstanceData.bIsTriggerPulled = IVRWeaponInterface::Execute_IsTriggerPulled(InstanceData.WeaponActor);
		}

		const UVRChamberComponent* ChamberComponent = nullptr;
		const UVRMagwellComponent* MagwellComponent = nullptr;
		const UVRInternalMagazineComponent* InternalMagComponent = nullptr;
		bool bIsWeaponBase = false;

		AVRWeaponBase* Weapon = Cast<AVRWeaponBase>(InstanceData.WeaponActor);
		if (Weapon)
		{
			bIsWeaponBase = true;
			ChamberComponent = Weapon->CachedChamberComponent.Get();
			if (!ChamberComponent)
			{
				ChamberComponent = Weapon->FindComponentByClass<UVRChamberComponent>();
				if (ChamberComponent)
				{
					Weapon->CachedChamberComponent = const_cast<UVRChamberComponent*>(ChamberComponent);
				}
			}

			MagwellComponent = Weapon->CachedMagwellComponent.Get();
			if (!MagwellComponent)
			{
				MagwellComponent = Weapon->FindComponentByClass<UVRMagwellComponent>();
				if (MagwellComponent)
				{
					Weapon->CachedMagwellComponent = const_cast<UVRMagwellComponent*>(MagwellComponent);
				}
			}

			InternalMagComponent = Weapon->FindComponentByClass<UVRInternalMagazineComponent>();

			// Calculate the delay in seconds (60 seconds / Rounds Per Minute)
			float FireRate = Weapon->GetCalculatedStats().FireRate;
			if (FireRate > 0.0f)
			{
				InstanceData.TimeBetweenShots = 60.0f / FireRate;
			}
		}
		else
		{
			ChamberComponent = InstanceData.WeaponActor->FindComponentByClass<UVRChamberComponent>();
			MagwellComponent = InstanceData.WeaponActor->FindComponentByClass<UVRMagwellComponent>();
			InternalMagComponent = InstanceData.WeaponActor->FindComponentByClass<UVRInternalMagazineComponent>();
		}

		// Populate chamber outputs
		if (ChamberComponent)
		{
			InstanceData.ChamberStateTag = ChamberComponent->GetChamberState();
			InstanceData.bHasRoundReady = ChamberComponent->IsRoundReady();
		}
		else
		{
			InstanceData.ChamberStateTag = FGameplayTag::EmptyTag;
			InstanceData.bHasRoundReady = false;
		}

		// Populate magazine/internal magazine outputs
		if (MagwellComponent)
		{
			InstanceData.bHasMagazineInserted = (MagwellComponent->AttachedMagazine != nullptr);
			InstanceData.MagazineAmmoCount = InstanceData.bHasMagazineInserted ? MagwellComponent->AttachedMagazine->CurrentAmmo : 0;
		}
		else if (InternalMagComponent)
		{
			InstanceData.bHasMagazineInserted = false;
			InstanceData.MagazineAmmoCount = InternalMagComponent->GetCurrentAmmoCount();
		}
		else
		{
			InstanceData.bHasMagazineInserted = false;
			InstanceData.MagazineAmmoCount = 0;
		}

		// Populate bHasAmmoRemaining (chamber has round, or other round providers have rounds)
		bool bHasAmmo = false;
		if (ChamberComponent && ChamberComponent->IsRoundReady())
		{
			bHasAmmo = true;
		}
		else
		{
			if (bIsWeaponBase && Weapon)
			{
				for (UActorComponent* Component : Weapon->CachedRoundProviders)
				{
					if (Component && !Component->IsA<UVRChamberComponent>())
					{
						if (Component->Implements<UVRRoundProvider>() && IVRRoundProvider::Execute_HasRound(Component))
						{
							bHasAmmo = true;
							break;
						}
					}
				}
			}
			else
			{
				TArray<UActorComponent*> RoundProviders;
				InstanceData.WeaponActor->GetComponents(RoundProviders);
				for (UActorComponent* Component : RoundProviders)
				{
					if (Component && Component->Implements<UVRRoundProvider>() && !Component->IsA<UVRChamberComponent>())
					{
						if (IVRRoundProvider::Execute_HasRound(Component))
						{
							bHasAmmo = true;
							break;
						}
					}
				}
			}
		}
		InstanceData.bHasAmmoRemaining = bHasAmmo;
	}	
}


