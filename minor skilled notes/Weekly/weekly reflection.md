# Week 1-5
these weeks mainly focused on completing a course from udemy on learning to use c++ in unreal engine, during the course i created 4 seperate small games. going from hand holding to doing most of the code yourselve. the first game was called obstacle assault, it was a simple game that explained the basics of working with c++. how to compline, inheritance and composition. it also talked about the importance of combining c++ and blueprints. the game was a small obbey game with moving and rotating platforms. 

the second game was called dungeon escape, this part foccused on using the input system and having scripts talk to each other. the game is a small escape room like experience whith a lock and key system. you have to pick up a little statue and place this on a plateau to open a secret door and get to the next similar puzzle.

the third game was called battleblasters, this game was a top view tank battle game where you control a small tank and have to shoot turrets, this part mainly foccused on explaining pointers and gave a deeper dive into the use of inheretance. During this project I also rebuilt a object pooling script I had made for unity to use in unreal engine. and used it to shoot the projectiles.

the final game was called shooter Sam. this part explored some more systems in unreal engine like animation blueprint, blend spaces. and ai, with blackboards and ai controllers. As this was the final project pretty mush all coding was up to you. the game was a third person shooter with cool sci-fi assets. featuring a assault rifle and ai.

# Week 6
This week I started on doing research for the design of my advanced gun [[System design/System design research|System design research]] and dessign patterns. after that began on making a chart ![[System design/System design chart (pre-development).canvas]] that aims to map out how my system will look and what component are going to be needed. And at the end I created a new project with the unreal vr template, that im going to improve using c++ to make (future) development easier. using a tutorial by yaro https://www.youtube.com/watch?v=nBKEMMbD3Pk&t=323s 

# Week 7
On Tuesday I have gotten a green light from Daniel [[QA 1]], we looked over my system design chart and agreed it looks like a strong starting point. I have also decided not to follow the course from yare as i am purely focusing on the weapon system and not the template. Today Wednesday march 25 i started in unreal, first i am going to work from the bottom up so I started creating the [[System design/documentation#Data Assets|data assets]] first, I had to learn to work with creating a plugin as its a little different than normal work. I also started working on the [[System design/documentation#UVRGrabComponent (Scene Component)|grab component]]. Thursday i spent 8 hours finishing the grab component, it works basically the same as unreal's standard grab component but better, as its now fully in c++ is completely decoupled and uses delegates and interfaces instead of hard casts

# Week 8
I spent the first two days further improving my grab and interaction component with better use of interfaces, it now not only sends a on grab event to what it grabbed but also broadcasts controller interactions like button presses and trigger values. which is great especially for my gun. after this i spent the rest of the week working on a [[System design/documentation#UVRChamberComponent (Scene Component)|chamber component]] because this holds a lot of logic and i thought it would be the hardest thing to make function correctly. while working on this I realised enums where a bit to stiff later users of the plugin would not be able to add, remove or change anything. so i started looking for an alternative and i found [[System design/documentation#Native Gameplay Tags|gameplay tags]]. with this i could essentially achieve the same functionality as with enums but it would be a lot more flexible.

# Week 9
On Monday i started working on the chamber component and eventually decided it was good enough for now. On Tuesday i only worked on the presentation for Wednesday and took the rest of the day off. the second presentation went a lot better than the first one. I practised the presentation and everything went a lot smoother and i was able to better explain my idea. Another thing im doing is creating a new version of the chart that shows how the system design turned out to be ![[System design/System design chart (in-development).canvas]]after the presentation was done my plan was to start on a actor base gun but i thought "if i make an actor wont i have the same inheritance pains as with a god class", so i started building a base gun scene component with the gun grip mesh the trigger mesh and muzzle mesh and bullet spawn point all in one. my idea was that this component would be a bare minimum gun with a Hit scan fire and later a spawned bullet fire. when i was done making the hit scan and tested the component it of course didn't work. putting all those meshes into one component meant they weren't independent but one component. So I decided the solution was to make a base weapon actor with no functionality except hold the meshes and provide components with data from the data asset. I moved the hit scan functionality to a fire component and ended the week. I think this design will be far more flexible and modular.

# Week 10
This week i tried to improve on the weapon base a bit, i wanted the code to set up the hierarchy automatically based on the data asset, this proved to be very complicated and hard. once it kind of worked i had issues with collision and physics so i decided i had to find the balance between what my plugin handles and what you have to do manually. in this case that meant creating static meshes in the blueprint yourselves and if the name matches the name in the data asset it will update to that asset. another thing i worked on (i already started working on this earlier ) was the chamber component. how its built right now is that it sets gameplay tags to certain states when things are happening, i did it this way because it was a bit easier to implement but i will update it to a [[Docs/State tree|state tree]].

# Week 11
I finished the state tree this Monday i wanted to make a feature where the data asset was automatically pushed to the state tree but was really struggling with it using SetContextDate was not working or didn't exist so i decided to create my own helper function for that purpose. When i tested the state tree on tuesday there were some issues in logic and how i understood the state tree, i came a long way but just couldnt find the mistake. So i tried fixing it with ai, this was a pretty bad idea because it ended up being worse and i was stupid eneugh to delete all my changes without having pushed the improvements i made. so i had some setback. I took a day off on wednesday and came back with a fresh pair of eyes on thursday i spent some time rebuilding my changes noticed some things that could be improved and eventually found the mistake. A [[Docs/State tree|state tree]] is heavaly hierarchal and the root task on start of the state tree transitions to the first task it can transition to this was the error causing my state tree to get in a loop. Taking a step back from a problem to return later can really help. 

# Week 12
This week i made some minor improvements mainly to improve my new data asset only logic. Other than that I mostly worked on my personal branding and portfolio redo this week

# Week 13
This week i started working on the slide of a gun, i realised slides, break open shotguns and triggers are essentially the same thing, they all change their transform rotational or linear so i will make one component that works for all those thing, the mechanical component. once i got the basic functionality down, grabbing it and sliding i got to integrating it correct events, delegates and working together with other components. this proved to be the hardest part and does not fully work yet. Now that the project is getting bigger its also getting harder to integrate well. 

# Week 14
This Monday, I finalized the slide mechanics. I successfully created a unified mechanical component that works almost perfectly for slides, triggers, and break-action shotguns. I also developed a specific StateTree task to animate these mechanical components (e.g., simulating blowback when fired, or locking/unlocking the slide).

However, when I made my first packaged build to test stability and performance, the weapons completely failed to render—they were entirely invisible, even though they worked perfectly in the Editor. After several hours of frustrating debugging and deep-diving into Unreal's documentation, I uncovered critical flaws in how my script constructed the guns. 

Initially, I used asynchronous loading and soft pointers (`TSoftObjectPtr`) to construct the weapon parts. I believed this was the most optimized approach, as it prevents missing references from breaking the game and avoids blocking the main thread. However, this caused three major engine-level issues in a packaged build:

### 1. The "Cooker" Problem
In the Editor, C++ has access to every file on the hard drive. In a **Packaged Build**, Unreal only includes (or "cooks") files it knows are needed. 
- **Hard Pointers** (`UStaticMesh*`) are automatically detected and cooked.
- **Soft Pointers** (`TSoftObjectPtr`) are **ignored** by the Cooker unless the Asset Manager is explicitly told to scan the Data Asset holding them.
Because my gun meshes were soft-referenced and not registered as Primary Assets, the Cooker assumed they were unused and left them out of the build entirely.

### 2. The Render Proxy Race Condition
Unreal optimizes rendering by creating lightweight "Proxies" for the GPU. 
- During synchronous construction, Unreal waits for the script to finish before creating all Proxies at once.
- With my asynchronous method, the Construction Script "finished" instantly, telling the engine the gun was ready to render *before* the meshes arrived. When the async callback finally fired a few frames later, it caused a race condition. Adding components dynamically during a heavy level-load can result in the actor's render state being locked or ignored by the initial visibility pass.

### 3. Construction Script Serialization Quirks
Even after fixing the Cooker issue by registering my Data Assets with the Asset Manager, the gun remained invisible. I discovered a notorious Unreal Engine quirk: **Packaged builds aggressively discard dynamically created components** generated during C++ `OnConstruction` if they aren't explicitly serialized into the map.

**The Solution:**
To resolve this, I overhauled the visual construction pipeline:
1. I configured the `DefaultGame.ini` to force the Asset Manager to scan and cook my `WeaponData`, `ProjectileData`, and `MagazineData` assets.
2. I switched to **synchronous loading** to prevent the render proxy race condition.
3. I bypassed the Construction Script serialization issue entirely by forcing the weapon to rebuild its visual components at runtime during `BeginPlay()`. By explicitly calling `AddInstanceComponent()`, I ensured the engine treats these dynamically spawned parts as persistent, runtime-instanced components. The weapon now renders perfectly in the packaged build!

### 4. Runtime Rebuild Race Conditions & Physics Explosions
Moving the component reconstruction to `BeginPlay()` solved the visibility issue but immediately introduced new bugs: grabbing the weapon stopped working, triggers failed to fire, and grabbing a sub-component (like the slide) attached the *entire* gun to the player's off-hand.
Through debugging, I learned:
- **Physics Desyncs:** Destroying and recreating collision components while the parent root is actively simulating physics causes severe overlap desyncs. The fix was to temporarily suspend the physics simulation during the `BeginPlay()` rebuild, safely swap the components, and then restore the simulation.
- **Component Initialization Order:** Because components spawned sequentially at runtime fire their `BeginPlay()` the moment they are registered, the Slide (`UVRMechanicalComponent`) was trying to find its child Grab Box before the Grab Box was even created. I fixed this by moving the discovery logic to a custom `InitializeComponent` interface function that guarantees it only runs *after* the entire weapon hierarchy is fully constructed.
- **Missing Data Asset Properties:** The "grab the slide and the whole gun moves" bug was caused by an oversight in the Data Asset structure. `bAttachOwnerOnGrab` was missing from the `UVRGrabSettings`, meaning dynamically spawned sub-components defaulted to `true`. Exposing this setting allowed me to properly configure slides to detach from the owner's transform and move independently.