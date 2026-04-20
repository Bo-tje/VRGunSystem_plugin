### 1. Architectural Pillar: Component-Based Composition

In the Blueprint version, I had what is referred to as a "God Actor" (BP_Gun) that handled shooting, reloading, and grabbing. In the C++ plugin, I am going to use the **Entity-Component Pattern**.

- **The Research:** Per the technical analysis, **Actor Components** are significantly more efficient than **Scene Components** for logic because they do not have a "Transform" (position/rotation). Using Scene Components only for physical parts (Slide, Mag) reduces the math the CPU performs for bounds and overlaps every frame.
    
- **Design Pattern: Composition over Inheritance.** Instead of creating a `Pistol` class and a `Rifle` class, you create a `WeaponActor` and "compose" it by adding a `SlideComponent` and a `MagwellComponent`.
    
- **System Reflection:** * **Main Weapon Actor:** Acts as the "Motherboard"
    
    - **Slide/Mag/Attachment:** Acts as physical **Scene Components** adding functionality to our Main Weapon actor.
        
    - **Recoil/FireLogic:** Acts as lightweight **Actor Components** adding additional logic.
        

---

### 2. Logic Pillar: The Finite State Machine (FSM)

VR reloading is non-linear. A player might pull a magazine out, drop it, put it back, or rack the slide halfway. Handling this with `if/else` branches in Blueprints leads to "spaghetti code."

- **The Research:** Modern Unreal development (and the CDPR report) suggests using **State Trees** or **C++ Enums** to define explicit states. This ensures that "Illegal Transitions" (like firing while the magazine is ejected) are physically impossible at the code level.
    
- **Design Pattern: State Pattern.** Each state (Idle, Firing, Reloading, Jammed) is a clear mode. When the player racks the slide, the system checks the current state; if the state is `LockedBack`, it transitions to `Idle`.
    
- **System Reflection:** To do this I will create  a `UENUM` called `EWeaponState`. Every input (Trigger Press, Grab) first asks: _"What is my current state?"_ before executing logic.
    

---

### 3. Interaction Pillar: Decoupled Interactors

The most common mistake in VR and development overall is "Hard Coupling"—the Gun looking for a "VR_Hand" and the Hand looking for a "Gun."

- **The Research:** The "Decoupling" feature mentioned in the CDPR collaboration report allows systems to evolve independently. If you decouple your grab logic, your plugin becomes "Interface-based," meaning it can work with _any_ VR pawn (Oculus, Vive, Index) without modification.
    
- **Design Pattern: Observer Pattern (Delegates).** The Hand doesn't "call" the Gun's fire function. The Hand "broadcasts" an event: `OnInteractionStarted`. The Gun "listens" for that event.
    
- **System Reflection:** Build a `UGrabbableComponent`. The Gun doesn't care _who_ is grabbing it; it just reacts when the `UGrabbableComponent` tells it, "A hand has picked us up."
    

---

### 4. Data Pillar: Data-Driven Configuration

You shouldn't write separate code for a Glock and an M4. The logic of "pulling a trigger" is the same; only the values (Damage, Fire Rate, Mesh) change.

- **The Research:** **Data Assets (`UDataAsset`)** are the "Lightweight" way to store configuration. Unlike Blueprints, which carry the weight of an entire class, Data Assets are pure data containers.
    
- **Design Pattern: Flyweight Pattern.** Instead of every Gun instance carrying its own heavy variables, they all point to a single `WeaponData` asset. This saves massive amounts of memory when you have many weapons in a scene.
    
- **System Reflection:** Creating a `UWeaponDataAsset`. It holds the `StaticMesh`, `FireRate`, `RecoilCurve`, and `MuzzleFX`. Your C++ code simply reads from this asset to decide how to behave.
    

---

### 5. Performance Pillar: Memory Management

In VR, maintaining 90fps is pretty much mandatory as it provides for a much better experience.

- **The Research:** Using **Structs (`USTRUCT`)** for high-frequency data (like bullet trajectories or hit results) is much faster than creating `UObjects`.
    
- **Design Pattern: Object Pooling.** Instead of `SpawnActor` and `DestroyActor` for every spent shell casing (which triggers the expensive Garbage Collector), you "Enable" and "Disable" pre-spawned actors.
    
- **System Reflection:** I will use the object pooling plugin i made to optimise frequently spawned objects like bullets
    

### Summary of Design Decisions

| System              | Logic Pattern  | Unreal Tool                   |
| ------------------- | -------------- | ----------------------------- |
| **Weapon Manager**  | [[System design/System design#Class Hierarchy|Composition]]    | `AActor` + Components         |
| **Reloading Logic** | [[Docs/State tree|State Machine]]  | `UENUM` or State Tree         |
| **Hand-to-Gun**     | [[Docs/Dynamic multicast delagates|Observer]]       | `Dynamic Multicast Delegates` |
| **Weapon Stats**    | [[System design/System design research#4. Data Pillar: Data-Driven Configuration|Flyweight]]      | `UDataAsset`                  |
| **Projectiles**     | Object Pooling | `My pooling plugin`           |


**Primary Engine Documentation**

- Epic Games. (2026). _Epic C++ coding standard for Unreal Engine 5.7_. Epic Developer Community. [https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)
    
- Epic Games. (2026). _Programming with C++ in Unreal Engine 5.7_. Epic Developer Community. [https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/programming-with-cplusplus-in-unreal-engine)
    
- Epic Games. (2026). _Reflection system in Unreal Engine 5.7_. Epic Developer Community. [https://dev.epicgames.com/documentation/en-us/unreal-engine/reflection-system-in-unreal-engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/reflection-system-in-unreal-engine)
    

**Technical Reports & Industry Case Studies**


- Jean-Paul Software. (2024). _Some of the things you didn't want to know about State Tree in UE5 and weren't afraid to ask_. Jean-Paul Software Dev Blog. [https://jeanpaulsoftware.com/2024/08/13/state-tree-hell/](https://jeanpaulsoftware.com/2024/08/13/state-tree-hell/)
    
- Rutherford, A. (2024). _CD Projekt RED on developers shaping UE advancement_. 80 Level. [https://80.lv/articles/cd-projekt-red-on-developers-shaping-ue-advancement](https://80.lv/articles/cd-projekt-red-on-developers-shaping-ue-advancement)
    

**Academic & Community Technical Analysis**

- Liu, B. (2023). _A deep dive into spawn actor ability task and Unreal Engine optimization_. Medium. [https://bobby-liu.medium.com/a-deep-dive-into-spawn-actor-ability-task-and-unreal-engine-optimization-153a1a8c7e91](https://bobby-liu.medium.com/a-deep-dive-into-spawn-actor-ability-task-and-unreal-engine-optimization-153a1a8c7e91)

