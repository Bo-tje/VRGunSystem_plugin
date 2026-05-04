### Class Hierarchy
Detailed technical specs in [[System design/documentation|Documentation]].
For architectural decisions, see [[System design/System design research|Research]].

- **`AVRWeaponBase` (Actor):** The core weapon container. Manages initialization of all modular components via `UVRWeaponData` and routes generic interaction events.
    
- **`UVRWeaponStateTreeComponent` (StateTree Component):** The "Brain." Drives weapon states (Idle, Firing, Empty, Jammed) using the Unreal State Tree and coordinates tasks like firing and chambering.
    
- **`UVRChamberComponent` (Scene Component):** Manages the round currently in the firing position. Evaluates whether a round is ready or empty.
    
- **`UVRFireComponent` (Scene Component):** Handles the physical and logical execution of firing a round, supporting both hitscan and projectile spawning, as well as applying recoil and haptics.
    
- **`UVRGrabComponent` (Scene Component):** Placed on any grabbable actor (like the weapon or magazine). Handles grabbing mechanics, socket snapping, and routing inputs.

- **`UVRInteractor` (Scene Component):** Placed on the VR hands/controllers. Detects grabbables and routes controller inputs directly into grabbed items.

- **`UVRCustomActionComponent` (Actor Component):** A modular action listener that hooks into interaction events based on Gameplay Tags to trigger arbitrary blueprint logic.
