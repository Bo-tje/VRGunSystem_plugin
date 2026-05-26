# VR Modular Weapon System Documentation

This document provides a comprehensive overview of the VR Modular Weapon System, its components, interfaces, and the State Tree-based logic.

## Table of Contents
1. [Core Components](#core-components)
2. [Data Assets](#data-assets)
3. [Interfaces](#interfaces)
4. [State Tree Integration](#state-tree-integration)
5. [Native Gameplay Tags](#native-gameplay-tags)

---

## Core Components

### `AVRWeaponBase` (Actor)
The base actor for all weapons in the system. It serves as a container that holds modular components and manages their initialization.

- **Key Properties:**
  - `WeaponData`: `UVRWeaponData*` - The configuration asset for this weapon.
  - `WeaponRoot`: `UBoxComponent*` - The physics root of the weapon. Shrunk to 1x1x1 core so dynamically attached parts can weld their collisions into a realistic bounding box.
  - `PartRoot`: `USceneComponent*` - The root for attaching modular weapon parts.
  - `StateTreeComponent`: `UVRWeaponStateTreeComponent*` - Handles the weapon's logic.
  - `CalculatedStats`: `FVRWeaponStats` - Dynamic struct storing all currently applied stat modifiers.
  - `CurrentFireModeIndex`: `int32` - Tracks the currently selected fire mode index.
  - `CurrentRecoilOffset` / `TargetRecoilOffset` / `RecoilVelocity`: `FRotator` - Real-time physics simulation variables for advanced procedural recoil.
- **Key Functions:**
  - `InitializeWeapon()`: Distributes `WeaponData` to all child components implementing `IVRWeaponComponentInterface`.
  - `ApplyWeaponDataVisuals()`: Dynamically spawns `UStaticMeshComponent`s and logical components based on `WeaponData`, snapping them via sockets and automatically enabling collision welding.
  - `UpdateCalculatedStats()`: Recalculates stats dynamically based on all attached modules and attachment points.
  - `GetDynamicComponentByName()`: Retrieves dynamically injected custom components.
  - `GetCurrentFireMode()`: Returns the currently active `FVRFireMode`.
  - `CycleFireMode(bool bBackward)`: Cycles through the available fire modes.
  - `SetFireModeIndex(int32 NewIndex)`: Directly sets the active fire mode index.
  - `Reload()`: Sends a reload state tree event and plays the configured `ReloadSound`.
- **Implementation:** Implements `IVRWeaponInterface` (including inputs, dry fires, and reloads) and `IVRInteractableInterface` for VR interaction routing.

### `UVRChamberComponent` (Scene Component)
Manages the round currently in the firing position.

- **Key Properties:**
  - `CurrentChamberState`: `FGameplayTag` - The current state (Empty, RoundReady, SpentCasing, Jammed).
  - `LoadedProjectile`: `UProjectileData*` - The actual round in the chamber.
  - `EjectedCasingClass`: `TSubclassOf<AVREjectedCasing>` - The physics casing actor to spawn when ejecting.
  - `EjectVelocityDirection` / `EjectVelocityStrength`: `FVector` / `float` - Local physics parameters driving casing ejection direction/impulse.
  - `BounceSoundsOverride`: `TArray<USoundBase*>` - Sound effect overrides for casing bounce hits.
- **Functions:**
  - `TryLoad(UProjectileData* NewRound)`: Attempts to load a round into the chamber.
  - `TryEject()`: Ejects whatever is currently in the chamber, spawning a physical casing actor in the process.
  - `TryGiveBullet()`: Used by the FireComponent to consume a round for firing.
  - `IsRoundReady()`: Returns true if there is a live round ready to fire.
  - `IsEmpty()`: Returns true if there is no round or casing in the chamber.
- **Delegates (Blueprint Assignable):**
  - `OnChamberStateChanged(FGameplayTag NewState)`
  - `OnRoundLoaded(UProjectileData* LoadedRound)`
  - `OnRoundFired(UProjectileData* FiredRound)`
  - `OnRoundEjected(UProjectileData* EjectedRound)`
- **Implementation:** Implements `IVRWeaponComponentInterface` and `IVRRoundProvider`.

### `UVRFireComponent` (Scene Component)
Handles the actual firing logic, including hitscan or projectile spawning. Modernized in V1 to spawn dynamic Niagara particles rather than legacy cascade systems.

- **Key Properties:**
  - `MuzzleSocketName`: `FName` - Socket on the mesh where shots originate.
  - `FireHapticScale` / `DryFireHapticScale`: `float` - Haptic feedback multipliers.
- **Functions:**
  - `HandleFiring(UProjectileData* ProjectileData)`: Executes the shot logic, spawning a Niagara muzzle flash and launching projectiles.
  - `HandleDryFire()`: Executes dry firing logic when no round is present in the chamber.
  - `GetMuzzleTransform()`: Returns the world transform of the muzzle socket.
- **Delegates (Blueprint Assignable):**
  - `OnFired()`: Triggered when a shot is successfully fired.
  - `OnDryFired()`: Triggered when the weapon attempts to fire but has no round.
- **Implementation:** Implements `IVRWeaponComponentInterface` and `IVRWeaponInterface`.

### `UVRWeaponFeedbackComponent` (Actor Component)
Decoupled feedback manager that centrally handles complex haptics, recoil patterns, and audio cues for the weapon.

- **Key Properties:**
  - `WeaponOwner`: `AVRWeaponBase*` - Reference to the weapon triggering feedback.
- **Functions:**
  - `PlayFiringFeedback()`: Triggers the dynamic recoil and haptic sequences defined by weapon stats.

### `UVRGrabComponent` (Box Component)
Manages the interaction between the weapon and the VR hands (Interactors). It inherits natively from `UBoxComponent`, allowing precise physical boundaries for grip areas.

- **Key Properties:**
  - `bIsHeld`: `bool` - Whether the component is currently being held.
  - `BoxExtents`: `FVector` - The precise dimensions of the grabbable area.
  - `MaxGrabDistance`: `float` - The maximum allowed distance from the *surface* of the box to successfully grab it (acts as a grab aura).
  - `GrabPriority`: `int32` - Defines which component is prioritized when multiple grabs overlap within their ranges.
  - `bUseSocketSnap`: `bool` - Snaps to `GrabSocketName` if true.
  - `GrabSocketName`: `FName` - The socket name used for snapping to the hand.
  - `ThrowMultiplier`: `float` - Multiplier for velocity when throwing.
  - `GrabPoseTag` / `HoverPoseTag`: `FGameplayTag` - Tags used to tell the Animation Blueprint which hand pose to use when hovering or grabbed.
  - `GrabHapticEffect`: `UHapticFeedbackEffect_Base*` - Haptic played on grab.
  - `GrabSound`: `USoundBase*` - Sound played on grab.
  - `HapticScale` / `bLoopHaptics`: `float` / `bool` - Determines how grabbing haptics are played.
  - `bShowDebugGizmos`: `bool` - Draws RGB coordinate system axes at Left/Right anchor offsets in PIE.
- **Functions:**
  - `TryGrab(UVRInteractor* Interactor)`: Attempts to grab the component with the given interactor.
  - `TryRelease()`: Releases the component from the current interactor.
  - `IsHeld()`: Returns true if held.
  - `GetHoldingHand()`: Returns the side (Left/Right) of the holding hand.
- **Delegates (Blueprint Assignable):**
  - `OnHoverStart` / `OnHoverEnd`
  - `StartAction` / `StopAction`
  - `OnTriggerStart` / `OnTriggerStop`
  - `OnPrimaryActionStart` / `OnPrimaryActionStop`
  - `OnSecondaryActionStart` / `OnSecondaryActionStop`
  - `OnGrabbed(AActor* InteractingHand)`
  - `OnReleased()`
- **Interfaces:** Implements `IVRInteractableInterface`.

### `UVRMechanicalComponent` (Scene Component)
Handles moving mechanical parts on the weapon, such as slides, triggers, or break-action hinges. It calculates hand offsets and drives component transforms purely via math (kinematic) to ensure perfect tracking without VR physics jitter.

- **Key Properties:**
  - `CurrentNormalisedValue`: `float` - The current state of the mechanical part (0.0 to 1.0).
  - `bIsLocked`: `bool` - Bypasses the return spring, allowing the component to be locked in place (e.g., slide lock on empty).
  - `RestingValue`: `float` - The target value for the return spring.
  - `MovementHapticEffect`: `UHapticFeedbackEffect_Base*` - Haptic effect triggered during movement limits.
  - `SlapVelocityThreshold`, `SlapReleaseDistanceThreshold`, `SlapMomentumThreshold`: `float` - Evaluates if a limit impact was a "Realistic Slap", preventing harsh clicking noises on slow manual movements.
  - `LocalAxis`: `FVector` - The local direction vector defining the mechanical path.
  - `bHasReturnSpring`: `bool` - Determines whether the mechanism naturally returns to RestingValue.
  - `bUseSimulatedInertia`: `bool` - Applies inertia properties allowing flicking or momentum transfer.
  - `LinkedComponent`: `FName` - Optional name of another mechanical component to drive synchronously.
  - `OnReachedMaxTag` / `OnReachedMinTag`: `FGameplayTag` - Event tags broadcasted at limits.
- **Functions:**
  - `SetNormalizedValue(float NewValue)`: Immediately updates the position and state of the component.
  - `SetIsLocked(bool bNewLocked)`: Locks or unlocks the return spring.
  - `SetRestingValue(float NewRestingValue)`: Dynamically changes the target state of the component (e.g., smoothly springing open).
  - `AddMomentum(float MomentumAmount)`: Injects simulated physical force (e.g., popping open a latch).
  - `UpdateFromHandLocation()`: Processes physics and kinematic offsets against the interactor.
  - `OnGrabbed()` / `OnReleased()`: Interaction lifecycle events for the mechanical grip.
- **Delegates (Blueprint Assignable):**
  - `OnReachedMax` / `OnReachedMin`
  - `OnValueChanged`
- **Implementation:** Implements `IVRWeaponComponentInterface`. It correctly calculates initial offsets via `CalculateRawHandValue` for both `Linear` (distance-based) and `Rotational` (angle-based with wrap-around safety) movement types.

### `UVRAttachmentPointComponent` (Scene Component)
Handles physical modular attachments by securely snapping and registering physical `AVRAttachmentActor` objects to the weapon. Snapping is deferred until the player releases the attachment inside the overlap radius.
- **Key Properties:**
  - `SocketName`: `FName` - The socket this point is bound to.
  - `CurrentAttachment`: `AVRAttachmentActor*` - The attachment currently slotted in.
- **Functions:**
  - `TryAttach()` / `Detach()`: Physically mounts or removes an attachment from the weapon. Updates weapon calculated stats.

### `AVRAttachmentActor` (Actor)
A physical, grabbable object representing an attachment (e.g. scopes, grips).
- **Key Properties:**
  - `StatModifier`: `UVRWeaponStatModifier*` - Modifiers injected into the weapon's `CalculatedStats` when attached.
  - `bIsHeld`: `bool` - True if the attachment is currently held. Snapping is ignored while this is true.

### `UVRMagwellComponent` (Scene Component)
Handles magazine detection, snapping logic, and ejection.
- **Key Properties:**
  - `MagazineSocketName`: `FName` - Where the magazine visually attaches.
  - `InsertRadius`: `float` - Maximum distance a magazine can be detected.
  - `CompatibleMagazinesTag`: `FGameplayTag` - Filters which magazines can be attached.
  - `AttachedMagazine`: `AVRMagazineBase*` - The current physical magazine attached.
  - `bEjectOnRelease`: `bool` - Automatically ejects the mag if the hand lets go before fully snapping.
  - `InsertSound`, `EjectSound`: `USoundBase*` - Audio cues for mag well interactions.
- **Functions:**
  - `OnMagazineGrabbed()`, `OnMagazineReleased()`, `EjectMagazine()`: Lifecycle events for magazine insertion.
- **Delegates:**
  - `OnMagazineAttached`, `OnMagazineDetached`

### `AVRMagazineBase` (Actor)
Physical grabbable magazine object that holds ammo and handles visual bullet meshes.
- **Key Properties:**
  - `MagazineData`: `UMagazineData*` - The data asset defining ammo count and types.
  - `CurrentAmmo`: `int32` - Currently remaining rounds.
  - `bShowVisualBullets`: `bool` - Toggle for rendering physical rounds at the top of the mag.
  - `GrabComponent`: `UVRGrabComponent*` - Core grab interaction component.
- **Functions:**
  - `RefillMagazine()`: Reloads the magazine to max capacity based on `MagazineData`.
  - `UpdateVisualBullets()`, `SetupVisualBullets()`: Procedurally renders individual rounds dropping as they are chambered.
- **Delegates:**
  - `OnAmmoEmpty`

### `UVRInternalMagazineComponent` (Scene Component)
Represents an internal tube magazine (e.g. for shotguns) or internal box magazine (e.g. for bolt-action/hunting rifles). It allows individual bullets or shells to be pushed manually into the weapon.
- **Key Properties:**
  - `MaxCapacity`: `int32` - The maximum number of rounds the internal magazine can hold.
  - `CompatibleAmmoTag`: `FGameplayTag` - Filters the types of ammunition compatible with this magazine.
  - `RequiredWeaponStateTags`: `FGameplayTagContainer` - Weapon state conditions required to allow manual insertion.
  - `LoadDetectionSphere`: `USphereComponent*` - Trigger volume detecting overlapping `AVRRoundActor`s.
  - `LoadSound` / `LoadHaptic`: Audio and haptic feedback played upon successful insertion.
  - `LoadedRounds`: `TArray<UProjectileData*>` - The internal ammunition stack.
- **Functions:**
  - `GetRound(UProjectileData*& OutRound)`: Pulls the next round from the magazine (implements `IVRRoundProvider` for integration with standard StateTree tasks).
  - `HasRound()`: Returns true if the magazine is not empty.
  - `OnOverlapBegin(...)`: Callback that detects and consumes an overlapping `AVRRoundActor`.

### `AVRProjectileBase` (Actor)
Physical bullet/projectile representation spawned into the world when the weapon is not using hitscan. Spawns Niagara system impact effects.
- **Key Properties:**
  - `ProjectileMovement`: `UProjectileMovementComponent*` - Drives the ballistics.
  - `CollisionComponent`: `USphereComponent*` - Physics boundaries.
- **Functions:**
  - `InitializeProjectile()`, `OnProjectileStop()`: Handles impact physics, spawning Niagara visual effects, and returns actor to pool after clearing owner/instigator references to avoid memory leaks.

### `AVREjectedCasing` (Actor)
Physics-driven shell casing spawned upon ejection.
- **Key Properties:**
  - `BounceSounds`: `TArray<USoundBase*>` - Random audio cues played on physics impact.
  - `LifeTime`: `float` - Duration in seconds before automatic fading and destruction.
- **Functions:**
  - `InitializeCasing(UStaticMesh* Mesh, FVector Impulse)`: Assigns the casing mesh and applies velocity force.

### `AVRRoundActor` (Actor)
Physical grabbable representation of a single live round or shell casing. It allows players to manually feed ammunition directly into a weapon's chamber or loading gate.
- **Key Properties:**
  - `ProjectileData`: `UProjectileData*` - The data asset specifying projectile behavior, compatibility, and visual mesh.
  - `GrabComponent`: `UVRGrabComponent*` - Roots the actor and enables VR grabbing/interaction.
  - `RoundMesh`: `UStaticMeshComponent*` - Renders the physical ammunition mesh (defaults to `ProjectileData->LiveRoundMesh`).

### `UVRInteractor` (Scene Component)
The component on the VR pawn/hand that initiates interactions with `UVRGrabComponent`.

- **Key Properties:**
  - `HandSide`: `EControllerHand` - Which hand this interactor represents.
  - `ActiveGrabComponent`: `UVRGrabComponent*` - The component currently being held.
  - `HoverHapticEffect`: `UHapticFeedbackEffect_Base*` - Haptic effect assigned to hover actions.
- **Functions:**
  - `IntendGrab()`: Attempts to grab the best nearby candidate.
  - `IntendRelease()`: Releases the current active grab.
  - `IntendActionStart(float Value, FGameplayTag ActionTag)`: Starts a specific action.
  - `IntendActionStop(FGameplayTag ActionTag)`: Stops the action.
  - `PlayHapticFeedback(UHapticFeedbackEffect_Base* Effect, float Scale, ...)`: Triggers hand haptics.
- **Interfaces:** Implements `IVRInteractorInterface`.

### `UVRCustomActionComponent` (Actor Component)
A utility component that listens to tagged interaction events to trigger custom blueprint logic.

- **Key Properties:**
  - `FilterTag`: `FGameplayTag` - Only routes actions matching this tag.
  - `CachedGrabComp`: `UVRGrabComponent*` - Automatically binds to the nearest grab component.
- **Delegates (Blueprint Assignable):**
  - `OnActionStart`
  - `OnActionStop`

### `UVRWeaponStateTreeComponent` (StateTree Component)
A specialized component that runs the State Tree logic for the weapon. It automatically binds `WeaponData` as context for the tree. See [[Docs/State tree|State Tree Overview]] for system details.

---

## Data Assets

### `UVRWeaponData`
Defines the base configuration for a weapon. See the [[System design/System design research#4. Data Pillar: Data-Driven Configuration|Data Pillar]] for the design rationale.
- `FireRate`: Shots per minute.
  - `bUseHitscan`: Toggle between hitscan and physical projectiles.
  - `RecoilAmount`: Scalar mapping for recoil implementation.
  - `BaseStats`: `FVRWeaponStats` - The default baseline RPG-like stats before component modifiers are applied.
  - `CompatibleMagazinesTag`: Filter tag for finding correct magazines.
  - `WeaponParts`: Array of `FVRWeaponPart`. Enforces purely data-driven structural construction using soft object pointers (`TSoftObjectPtr<UStaticMesh>`) and explicit Sockets/Offsets to bypass the blueprint editor.
  - `AdditionalComponents`: Array of `FVRWeaponDynamicComponent` for dynamically injecting arbitrary extra Actor Components (like `UVRGrabComponent` or logic nodes) directly from the Data Asset.
  - `DefaultProjectile`: The standard projectile this weapon uses.
  - `FireSound`, `DryFireSound`, `ReloadSound`, `MuzzleFlash` (Niagara), `FireHapticEffect`.

### Component Settings Architecture
Settings for dynamically injected components are managed via subclasses of `UVRWeaponComponentSettings`, which are instantiated directly within `UVRWeaponData` (inside the `AdditionalComponents` array):
- **`UVRGrabSettings`**: Configures grab haptics, throw multipliers, snap spheres, `BoxExtents`, `MaxGrabDistance`, `GrabPriority`, `GrabSound`, and the `GrabPoseTag`/`HoverPoseTag` for animation routing.
- **`UVRFireSettings`**: Defines muzzle socket names, specific fire/dry-fire haptic scaling, `FireModes` (array of `FVRFireMode`), `RoundsPerMinute`, `BurstCount`, and `bIsAutomatic`.
- **`UVRMechanicalSettings`**: Fully configures a mechanical part. Includes `MechanicalMovementType` (Linear/Rotational), `LocalAxis`, `MaxRange`, `bHasReturnSpring`, `RestingValue`, `HapticTickThreshold`, physics inertia settings (`bUseSimulatedInertia`, `InertiaMultiplier`), realistic slap impact thresholds, and `LinkedComponent`.
- **`UVRMagwellSettings`**: Defines `MagazineSocketName`, `CompatibleMagazinesTag`, `InsertRadius`, `bEjectOnRelease`, and magwell-specific sounds/haptics (`InsertSound`, `EjectSound`, `InsertHapticEffect`).

### `UProjectileData`
Defines the properties of a bullet/round.
- `AmmoTags`: Identifying gameplay tags for compatibility.
- `Damage` / `HitscanDamage`: Damage dealt on impact (based on execution type).
- `InitialSpeed` / `GravityScale`: Properties for physics projectiles.
- `ProjectileClass`: The spawnable actor when using physical projectiles.
- `HitscanRange`: Distance check for raycast variants.
- `LiveRoundMesh` / `SpentCasingMesh`: Visual representations.
- `ImpactEffect` / `ImpactSound`: Visual/Audio played on hit.
- `MuzzleFlashOverride` / `FireSoundOverride`: Potential overrides when fired.
- `PelletCount`: Number of pellets fired per shot (defaults to 1).
- `SpreadAngle`: Cone spread angle in degrees (defaults to 0.0).

### `UMagazineData`
Defines the base properties of a magazine.
- `MaxAmmo`: Maximum rounds the magazine can hold.
- `MagazineType`: Gameplay tag defining compatibility.
- `MagazineMesh`: The static mesh representing the physical magazine.

### `FVRWeaponStats`
A structural grouping for defining and dynamically modifying weapon characteristics (RPG-like stat system).
- **Properties:** `FireRate`, `FireRateOffset`, `RecoilMultiplier`, `DamageMultiplier`, `ReloadSpeedMultiplier`, `BulletVelocityMultiplier`, `SpreadMultiplier`, `PelletCountOffset`.
- **Advanced Recoil Simulation:** `RecoilYaw`, `RecoilPitch`, `RecoilSpringStiffness`, `RecoilSpringDamping`.
- **Overrides:** `MuzzleFlashOverride`, `FireSoundOverride`.

---

## Interfaces

### `IVRWeaponInterface`
Implemented by the weapon actor and components that react to trigger/button inputs.
- `PullTrigger()` / `ReleaseTrigger()`: Handle trigger state changes.
- `PrimaryAction()` / `ReleasePrimaryAction()`
- `SecondaryAction()` / `ReleaseSecondaryAction()`
- `IsTriggerPulled()`: Query current trigger state.

### `IVRWeaponComponentInterface`
Implemented by components that need setup from the weapon.
- `InitializeComponent(UVRWeaponData* InData)`: Called when the weapon is initialized.

### `IVRRoundProvider`
Implemented by components that can provide a round (e.g., Chamber, Magazine).
- `GetRound(UProjectileData*& OutRound)`: Returns true if it could provide a round.

### `IVRInteractableInterface`
Implemented by objects that react to generic VR input and grab mechanisms.
- `StartAction(Interactor, ActionValue, ActionTag)` / `StopAction(...)`: Routes action tags and values to components.
- `OnHoverStart(Interactor)` / `OnHoverEnd(Interactor)`: Triggers when the VR hand approaches/leaves.

### `IVRInteractorInterface`
Defines properties of the interactor.
- `GetProvidingPlayerController()`: Returns associated controller.

---

## State Tree Integration

The weapon logic is driven by a State Tree using the `VR Weapon State Tree Schema`.

### Evaluators
- **`FSTEval_Weapon`**: Monitors the weapon state.
  - **Outputs:**
    - `bIsTriggerPulled`: From `IVRWeaponInterface`.
    - `bHasRoundReady`: From `UVRChamberComponent`.
    - `ChamberStateTag`: Current gameplay tag of the chamber.

### Conditions
- **`FSTCondition_FireMode`**: Evaluates whether the current active Fire Mode matches specific rules (e.g., `bCheckIsAutomatic`, `bCheckModeName`).
- **`FSTCondition_MechanicalState`**: Evaluates the boolean state (`bIsHeld`, `bIsLocked`) of a mechanical component, routing to the primary mechanical part if no specific component name is provided.

### Tasks
- **`FSTTask_FireWeapon`**:
  - Attempts to fire a round.
  - Parameter: `bOnlyFireFromChamber` - If true, won't pull directly from a magazine.
- **`FSTTask_EjectRound`**:
  - Calls `TryEject()` on the chamber.
- **`FSTTask_EjectMag`**:
  - Automatically triggers the `EjectMagazine()` logic on the weapon's `UVRMagwellComponent`.
- **`FSTTask_ChamberRound`**:
  - Searches for an `IVRRoundProvider` (like a Magazine) and attempts to move a round into the `UVRChamberComponent`.
  - Parameter: `bInfiniteAmmo` - Option to chamber indefinitely.
- **`FSTTask_PlayWeaponFeedback`**:
  - Executes dynamic haptics and audio decoupled from firing logic. Supports overrides (`HapticOverride`, `SoundOverride`).
  - Parameter: `bIsReload` - Play reload sound and silence muzzle flash.
- **`FSTTask_AnimateMechanical`**:
  - Procedurally applies momentum or sets resting states for a specified `UVRMechanicalComponent` (e.g., locking the slide back on empty).
- **`FSTTask_CycleFireMode`**:
  - Cycles or sets the specific fire mode index on the weapon actor.

---

## Editor Tooling

The plugin contains a dedicated Unreal Engine editor module (`VRModularWeaponSystemEditor`) that provides custom Slate-based asset editor tooling to streamline configuring weapons and analyzing their stats.

### `FVRWeaponDataAssetEditor` (Asset Editor Toolkit)
A custom editor toolkit that opens when double-clicking a `UVRWeaponData` asset. It organizes the interface into three main docking tabs:
- **Properties Tab:** Houses the standard Details view for editing properties, settings, components, and parts lists.
- **Live 3D Preview Tab (`SVRWeaponPreviewViewport`):** Uses an advanced preview scene to assemble and render the modular meshes defined in the data asset. Updates and centers the camera automatically whenever properties change.
- **Stat Dashboard Tab (`SVRWeaponStatVisualizer`):** Shows comparative stats (Base Stats vs. Final Stats after dynamic modifiers from attachments/parts are computed). Features styled progress bars and performance deltas.

---

## Native Gameplay Tags

Defined in `VRNativeTags.h`:
- **Input:** `Trigger`, `PrimaryInput`, `SecondaryInput`, `Reload`, `ReloadReleased`
- **Weapon states:** `Idle`, `Firing`, `Reloading`, `Empty`, `Jammed`
- **Chamber states:** `Chamber_Empty`, `Chamber_RoundReady`, `Chamber_SpentCasing`, `Chamber_Jammed`

---

## Editor Gameplay Tag Restrictions

To improve developer UX and prevent configuration errors, several editable `FGameplayTag` and `FGameplayTagContainer` fields are restricted using the `Categories` metadata specifier. This ensures that the details panel dropdown only displays valid tags in context:

| Component / Asset | Property | Restricted Category | Description |
| :--- | :--- | :--- | :--- |
| **`UVRWeaponComponentSettings`** (and subclasses) | `BindToInputTags` | `VRModularWeaponSystem.Interaction` | Maps controller input actions to mechanical actions. |
| **`UVRGrabComponent`** / **`UVRGrabSettings`** | `GrabPoseTag` / `HoverPoseTag` | `VRModularWeaponSystem.AnimPose` | Tells the animation blueprint which hand poses to play. |
| **`UVRMechanicalComponent`** / **`UVRMechanicalSettings`** | `OnReachedMaxTag` / `OnReachedMinTag` | `VRModularWeaponSystem.Event` | Broadcasts events when mechanical parts reach limits. |
| **`UVRMagwellComponent`** / **`UVRMagwellSettings`** | `CompatibleMagazinesTag` | `VRModularWeaponSystem.Magazine` | Defines which magazine types can be inserted. |
| **`UMagazineData`** | `MagazineType` | `VRModularWeaponSystem.Magazine` | Self-identifying tag of the magazine asset. |
| **`UProjectileData`** | `AmmoTags` | `VRModularWeaponSystem.Ammo` | Self-identifying tag of the ammunition type. |
| **`UVRChamberComponent`** / **`UVRInternalMagazine`** | `CompatibleAmmoTag` | `VRModularWeaponSystem.Ammo` | Filters which ammunition types can be loaded. |
| **`UVRChamberComponent`** / **`UVRInternalMagazine`** | `RequiredWeaponStateTags` | `VRModularWeaponSystem.State` | Restricts manual loading to specific weapon states. |
| **`UVRWeaponData`** | `InputTagToComponentName` (Keys) | `VRModularWeaponSystem.Interaction` | Maps input actions directly to component names. |

