#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Data/VRWeaponStats.h"
#include "VRWeaponData.generated.h"

/** Base class for custom component settings overridden in the Data Asset. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, Abstract)
class VRMODULARWEAPONSYSTEM_API UVRWeaponComponentSettings : public UObject
{
	GENERATED_BODY()
	
	
};

/** Specific settings for VR Grab Components. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class VRMODULARWEAPONSYSTEM_API UVRGrabSettings : public UVRWeaponComponentSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	UHapticFeedbackEffect_Base* GrabHapticEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float HapticScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float ThrowMultiplier = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	bool bUseSocketSnap = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float SphereRadius = 12.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float BreakDistance = 25.0f;

	/** A tag that can be read by the Interactor's Animation Blueprint to trigger a specific hand pose. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	FGameplayTag AnimationGrabPoseTag;

	/** If true, this grip is considered the main handle for haptics scaling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	bool bIsMainGrip = false;
};

/** Specific settings for the Mechanical component */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class VRMODULARWEAPONSYSTEM_API UVRMechanicalSettings : public UVRWeaponComponentSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical", meta = (Categories = "VRModularWeaponSystem.MechanicalMovement"))
	FGameplayTag MechanicalMovementType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	FVector LocalAxis;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	float MaxRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	bool bInvertDirection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | State")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Physics")
	bool bUseSimulatedInertia = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Physics", meta = (EditCondition = "bUseSimulatedInertia"))
	float InertiaMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	bool bHasReturnSpring;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical", meta = (EditCondition = "bHasReturnSpring"))
	float ReturnSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	float RestingValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	TObjectPtr<UHapticFeedbackEffect_Base> MovementHapticEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float HapticTickThreshold = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Events")
	FGameplayTag OnReachedMaxTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Events")
	FGameplayTag OnReachedMinTag;
};


/** Specific settings for VR Fire Components (Muzzles). */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class VRMODULARWEAPONSYSTEM_API UVRFireSettings : public UVRWeaponComponentSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Settings")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Settings")
	float FireHapticScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Settings")
	float DryFireHapticScale = 0.5f;
};

class UStateTree;
class UProjectileData;

USTRUCT(BlueprintType)
struct FVRWeaponPart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	FName PartName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Sockets")
	FName ParentSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Sockets")
	FTransform PartOffset;
};

USTRUCT(BlueprintType)
struct FVRWeaponDynamicComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	FName ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	TSubclassOf<UActorComponent> ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Overrides")
	TSoftObjectPtr<UStaticMesh> OptionalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Physics")
	bool bWeldCollision = false;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Component|Overrides")
	TObjectPtr<UVRWeaponComponentSettings> Settings;

	/** Stat modifiers provided by this component (for attachments like vertical grips/suppressors). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Overrides")
	TObjectPtr<UVRWeaponStatModifier> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Sockets")
	FName ParentSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component|Sockets")
	FTransform RelativeOffset;
};

/**
 * UVRWeaponData defines the configuration for a weapon.
 */
UCLASS(BlueprintType)
class VRMODULARWEAPONSYSTEM_API UVRWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	FString WeaponName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Weapon | Logic")
	TObjectPtr<UStateTree> StateTree;

	/** Maps Interaction Tags to Component Names for flexible input routing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Weapon | Logic")
	TMap<FGameplayTag, FName> InputTagToComponentName;

	// --- Base Stats ---
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	FVRWeaponStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	bool bUseHitscan = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	FGameplayTag CompatibleMagazinesTag;
	
	/** The default projectile of this weapon fires when no other projectile is provided. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	TObjectPtr<UProjectileData> DefaultProjectile;


	// --- Modular Parts ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Composition")
	TArray<FVRWeaponPart> WeaponParts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Composition")
	TArray<FVRWeaponDynamicComponent> AdditionalComponents;


	// --- Visual FX ---
	
	/** If true, the weapon automatically plays FireSound, MuzzleFlash, and Haptics when fired. If false, you must trigger them manually (e.g. via StateTree tasks). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	bool bAutoPlayWeaponFeedback = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> DryFireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> ReloadSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHapticEffect;
	
};

