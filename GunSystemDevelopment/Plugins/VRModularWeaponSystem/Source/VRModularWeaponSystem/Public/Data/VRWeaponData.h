#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "Data/VRWeaponStats.h"
#include "VRWeaponData.generated.h"

UENUM(BlueprintType)
enum class EMechanicalMovementType : uint8
{
	Linear,
	Rotational
};


/** Base class for custom component settings overridden in the Data Asset. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced, Abstract)
class VRMODULARWEAPONSYSTEM_API UVRWeaponComponentSettings : public UObject
{
	GENERATED_BODY()
	
public:
	/** Any input tags added here will automatically be routed to this component when the weapon receives them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (Categories = "VRModularWeaponSystem.Interaction"))
	FGameplayTagContainer BindToInputTags;
};

/** Specific settings for VR Grab Components. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class VRMODULARWEAPONSYSTEM_API UVRGrabSettings : public UVRWeaponComponentSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	TObjectPtr<UHapticFeedbackEffect_Base> GrabHapticEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Audio")
	TObjectPtr<USoundBase> GrabSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float HapticScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float ThrowMultiplier = 1.2f;

	/** Number of frames of velocity history to average when throwing the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	int32 ThrowVelocityBufferHistory = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	bool bUseSocketSnap = false;

	/** Maximum allowed distance from the surface of this box to successfully grab it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float MaxGrabDistance = 12.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	float BreakDistance = 25.0f;

	/** A tag that can be read by the Interactor's Animation Blueprint to trigger a specific hand pose. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings", meta = (Categories = "VRModularWeaponSystem.AnimPose"))
	FGameplayTag AnimationGrabPoseTag;

	/** A tag for the Animation Blueprint to trigger a hand pose when hovering. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings", meta = (Categories = "VRModularWeaponSystem.AnimPose"))
	FGameplayTag AnimationHoverPoseTag;

	/** Higher priority grabs take precedence when multiple components overlap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	int32 GrabPriority = 0;

	/** Any input tags added here will trigger the specified State Tree Event directly on the weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Routing")
	TMap<FGameplayTag, FGameplayTag> StateTreeEventRouting;

	/** The exact dimensions of the grabbable area. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Collision")
	FVector BoxExtents = FVector(5.0f, 5.0f, 5.0f);

	/** If true, this grip is considered the main handle for haptics scaling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	bool bIsMainGrip = false;

	/** If true, the entire weapon will attach to the hand. Must be FALSE for slides, bolts, and pumps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings")
	bool bAttachOwnerOnGrab = true;

	/** Default/fallback socket on the weapon mesh. Empty = use GrabComponent transform. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Socket")
	FName GripSocketName;

	/** Optional: Name of a socket on a parent StaticMesh to use when grabbed by the right hand.
	  * If empty, the system will fall back to GripSocketName. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Socket")
	FName RightHandGripSocketName;

	/** Optional: Name of a socket on a parent StaticMesh to use when grabbed by the left hand.
	  * If empty, the system will fall back to GripSocketName. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Socket")
	FName LeftHandGripSocketName;

	/** Default rotation offset applied after socket snap. Fine-tunes weapon orientation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Socket")
	FRotator GripRotationOffset = FRotator::ZeroRotator;

	/** Right hand rotation offset. Overrides GripRotationOffset for right hand if not zero. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Socket")
	FRotator RightHandRotationOffset = FRotator::ZeroRotator;

	/** Left hand rotation offset. Overrides GripRotationOffset for left hand if not zero. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Socket")
	FRotator LeftHandRotationOffset = FRotator::ZeroRotator;

	/** Smooth grab: weapon lerps into hand instead of instant snap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Feel")
	bool bUseSmoothGrab = false;

	/** Speed of smooth grab lerp. Higher = faster snap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab Settings | Feel")
	float GrabLerpSpeed = 15.0f;
};

/** Specific settings for the Mechanical component */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class VRMODULARWEAPONSYSTEM_API UVRMechanicalSettings : public UVRWeaponComponentSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	EMechanicalMovementType MechanicalMovementType = EMechanicalMovementType::Linear;

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

	/** Optional name of another mechanical component on this weapon to drive when this component moves. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical")
	FName LinkedComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Haptics")
	TObjectPtr<UHapticFeedbackEffect_Base> LimitReachedHapticEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Events", meta = (Categories = "VRModularWeaponSystem.Event"))
	FGameplayTag OnReachedMaxTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Events", meta = (Categories = "VRModularWeaponSystem.Event"))
	FGameplayTag OnReachedMinTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Audio")
	TObjectPtr<USoundBase> CockedSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Audio")
	TObjectPtr<USoundBase> SlapSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Audio")
	TObjectPtr<USoundBase> MovementSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Haptics")
	TObjectPtr<UHapticFeedbackEffect_Base> MovementHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Haptics")
	float HapticTickThreshold = 2.0f;

	/** Minimum hand velocity required to play the limit reached sound/haptics when held. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Audio")
	float SlapVelocityThreshold = 1.5f;

	/** Minimum distance (normalized 0-1) the component must be released from to play limit reached sound/haptics on spring return. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Audio")
	float SlapReleaseDistanceThreshold = 0.1f;

	/** Minimum simulated momentum required to play limit reached sound/haptics. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Audio", meta = (EditCondition = "bUseSimulatedInertia"))
	float SlapMomentumThreshold = 0.5f;

	/** Normalized distance threshold beyond slide lock required to trigger a slide release when pulling further back. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | State")
	float LockReleaseThreshold = 0.03f;

	/** Friction deceleration coefficient for simulated mechanical inertia. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Physics", meta = (EditCondition = "bUseSimulatedInertia"))
	float InertiaFriction = 8.0f;

	/** Physics inertia linear sensitivity scaling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Physics", meta = (EditCondition = "bUseSimulatedInertia"))
	float InertiaLinearSensitivity = -0.0005f;

	/** Physics inertia rotational sensitivity scaling. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Physics", meta = (EditCondition = "bUseSimulatedInertia"))
	float InertiaRotationalSensitivity = -0.001f;

	/** Maximum parent motion acceleration cap clamped for inertia calculations. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mechanical | Physics", meta = (EditCondition = "bUseSimulatedInertia"))
	float InertiaMaxAcceleration = 20000.0f;
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

/** Specific settings for Magwell Components. */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class VRMODULARWEAPONSYSTEM_API UVRMagwellSettings : public UVRWeaponComponentSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings")
	FName MagazineSocketName = TEXT("Magwell");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings")
	FName DropZoneSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings")
	bool bEjectOnRelease = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings")
	float GrabRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings")
	float InsertRadius = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings", meta = (Categories = "VRModularWeaponSystem.MagazineType"))
	FGameplayTag CompatibleMagazinesTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings | Feedback")
	TObjectPtr<USoundBase> InsertSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings | Feedback")
	TObjectPtr<USoundBase> EjectSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings | Feedback")
	TObjectPtr<UHapticFeedbackEffect_Base> HoverHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings | Feedback")
	TObjectPtr<UHapticFeedbackEffect_Base> InsertHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Magwell Settings | Feedback")
	TObjectPtr<UHapticFeedbackEffect_Base> EjectHapticEffect;
};

class UStateTree;
class UProjectileData;

USTRUCT(BlueprintType)
struct FVRFireMode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode")
	FName ModeName = TEXT("Single");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode")
	bool bIsAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode")
	int32 BurstCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fire Mode")
	float RoundsPerMinute = 600.0f;
};

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Physics")
	bool bWeldCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Part|Physics")
	FName CollisionProfile = TEXT("PhysicsBody");
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
class VRMODULARWEAPONSYSTEM_API UVRWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
	FString WeaponName;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VR Weapon | Logic")
	TObjectPtr<UStateTree> StateTree;

	// --- Base Stats ---
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	FVRWeaponStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	bool bUseHitscan = false;
	
	/** The default projectile of this weapon fires when no other projectile is provided. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	TObjectPtr<UProjectileData> DefaultProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default Weapon Stats")
	TArray<FVRFireMode> FireModes;


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
	TObjectPtr<UNiagaraSystem> MuzzleFlash;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<UNiagaraSystem> ChamberSmoke;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> DryFireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<USoundBase> ReloadSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Visuals")
	TObjectPtr<UHapticFeedbackEffect_Base> FireHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta=(GetOptions="GetAvailableComponentNames", Categories="VRModularWeaponSystem.Interaction"))
	TMap<FGameplayTag, FName> InputTagToComponentName;

	UFUNCTION(CallInEditor)
	TArray<FName> GetAvailableComponentNames() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

};

