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
- **Key Functions:**
  - `InitializeWeapon()`: Distributes `WeaponData` to all child components implementing `IVRWeaponComponentInterface`.
  - `ApplyWeaponDataVisuals()`: Dynamically spawns `UStaticMeshComponent`s and logical components based on `WeaponData`, snapping them via sockets and automatically enabling collision welding.
- **Implementation:** Implements `IVRWeaponInterface` to handle primary/secondary inputs and weapon state, as well as `IVRInteractableInterface` for VR interaction routing.

### `UVRChamberComponent` (Scene Component)
Manages the round currently in the firing position.

- **Key Properties:**
  - `CurrentChamberState`: `FGameplayTag` - The current state (Empty, RoundReady, SpentCasing, Jammed).
  - `LoadedProjectile`: `UProjectileData*` - The actual round in the chamber.
- **Functions:**
  - `TryLoad(UProjectileData* NewRound)`: Attempts to load a round into the chamber.
  - `TryEject()`: Ejects whatever is currently in the chamber (returns the ejected projectile).
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
Handles the actual firing logic, including hitscan or projectile spawning.

- **Key Properties:**
  - `MuzzleSocketName`: `FName` - Socket on the mesh where shots originate.
  - `FireHapticScale` / `DryFireHapticScale`: `float` - Haptic feedback multipliers.
- **Functions:**
  - `HandleFiring(UProjectileData* ProjectileData)`: Executes the shot logic using the provided projectile data.
  - `HandleDryFire()`: Executes dry firing logic when no round is present in the chamber.
  - `GetMuzzleTransform()`: Returns the world transform of the muzzle socket.
- **Delegates (Blueprint Assignable):**
  - `OnFired()`: Triggered when a shot is successfully fired.
  - `OnDryFired()`: Triggered when the weapon attempts to fire but has no round.
- **Implementation:** Implements `IVRWeaponComponentInterface` and `IVRWeaponInterface`.

### `UVRGrabComponent` (Scene Component)
Manages the interaction between the weapon and the VR hands (Interactors).

- **Key Properties:**
  - `bIsHeld`: `bool` - Whether the component is currently being held.
  - `bUseSocketSnap`: `bool` - Snaps to `GrabSocketName` if true.
  - `GrabSocketName`: `FName` - The socket name used for snapping to the hand.
  - `ThrowMultiplier`: `float` - Multiplier for velocity when throwing.
  - `GrabPoseTag`: `FGameplayTag` - A tag used to tell the Animation Blueprint which hand pose to use when grabbed.
  - `GrabHapticEffect`: `UHapticFeedbackEffect_Base*` - Haptic played on grab.
  - `HapticScale` / `bLoopHaptics`: `float` / `bool` - Determines how grabbing haptics are played.
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
  - `CurrentMomentum`: `float` - Stores simulated physics force.
- **Functions:**
  - `SetNormalizedValue(float NewValue)`: Immediately updates the position and state of the component.
  - `SetIsLocked(bool bNewLocked)`: Locks or unlocks the return spring.
  - `SetRestingValue(float NewRestingValue)`: Dynamically changes the target state of the component (e.g., smoothly springing open).
  - `AddMomentum(float MomentumAmount)`: Injects simulated physical force (e.g., popping open a latch).
- **Delegates (Blueprint Assignable):**
  - `OnReachedMax` / `OnReachedMin`
  - `OnValueChanged`
- **Implementation:** Implements `IVRWeaponComponentInterface`. It correctly calculates initial offsets via `CalculateRawHandValue` for both `Linear` (distance-based) and `Rotational` (angle-based with wrap-around safety) movement types.

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
- `CompatibleMagazinesTag`: Filter tag for finding correct magazines.
- `WeaponParts`: Array of `FVRWeaponPart`. Enforces purely data-driven structural construction using soft object pointers (`TSoftObjectPtr<UStaticMesh>`) and explicit Sockets/Offsets to bypass the blueprint editor.
- `AdditionalComponents`: Array of `FVRWeaponDynamicComponent` for dynamically injecting arbitrary extra Actor Components (like `UVRGrabComponent` or logic nodes) directly from the Data Asset.
- `DefaultProjectile`: The standard projectile this weapon uses.
- `FireSound`, `DryFireSound`, `ReloadSound`, `MuzzleFlash`, `FireHapticEffect`.

### Component Settings Architecture
Settings for dynamically injected components are managed via subclasses of `UVRWeaponComponentSettings`, which are instantiated directly within `UVRWeaponData` (inside the `AdditionalComponents` array):
- **`UVRGrabSettings`**: Configures grab haptics, throw multipliers, snap spheres, break distances, and the `GrabPoseTag` for animation routing.
- **`UVRFireSettings`**: Defines muzzle socket names and specific fire/dry-fire haptic scaling.
- **`UVRMechanicalSettings`**: Fully configures a mechanical part. Includes `MechanicalMovementType` (Linear/Rotational), `LocalAxis`, `MaxRange`, `bHasReturnSpring`, `RestingValue`, `HapticTickThreshold`, and physics inertia settings (`bUseSimulatedInertia`, `InertiaMultiplier`).

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

### `UMagazineData`
Defines the base properties of a magazine.
- `MaxAmmo`: Maximum rounds the magazine can hold.
- `MagazineType`: Gameplay tag defining compatibility.
- `MagazineMesh`: The static mesh representing the physical magazine.

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

### Tasks
- **`FSTTask_FireWeapon`**:
  - Attempts to fire a round.
  - Parameter: `bOnlyFireFromChamber` - If true, won't pull directly from a magazine.
- **`FSTTask_EjectRound`**:
  - Calls `TryEject()` on the chamber.
- **`FSTTask_ChamberRound`**:
  - Searches for an `IVRRoundProvider` (like a Magazine) and attempts to move a round into the `UVRChamberComponent`.
  - Parameter: `bInfiniteAmmo` - Option to chamber indefinitely.

---

## Native Gameplay Tags

Defined in `VRNativeTags.h`:
- **Input:** `Trigger`, `PrimaryInput`, `SecondaryInput`
- **Weapon states:** `Idle`, `Firing`, `Reloading`, `Empty`, `Jammed`
- **Chamber states:** `Chamber_Empty`, `Chamber_RoundReady`, `Chamber_SpentCasing`, `Chamber_Jammed`
