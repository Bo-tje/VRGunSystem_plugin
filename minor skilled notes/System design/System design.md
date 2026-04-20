### Class Hierarchy
Detailed technical specs in [[System design/documentation|Documentation]].
For architectural decisions, see [[System design/System design research|Research]].

- **`UVRWeaponComponent` (Actor Component):** The "Brain." It manages weapon states (Idle, Firing, Reloading, Jammed) and coordinates the other components.
    
- **`UVRSlideComponent` (Scene Component):** Handles the physical movement of the slide, back-locking, and manual racking.
    
- **`UVRMagwellComponent` (Scene Component):** Handles magazine detection, snapping logic, and ejection.
    
- **`UVRFireLogicComponent` (Actor Component):** Pure math/logic for fire rate, recoil curves, and projectile spawning.
    
- **`UVRGrabbableComponent` (Actor Component):** Handles grabbing objects in vr.
