
## Iain Douglas Week 3
The original idea sound a bit ambitious, there are a lot of possible assets and your just one person. This game might not be the right fit for showing of your c++ skills. Think in terms of mechanics and what could be cool and fun to make instead of making mechanics around a story.  Look at it more from a Engineering point of view than a Designer point of view.
focus on engagement loops - gameplay loops - economy

## Daniel Valente de Macedo Week 3
We came up with a new idea while talking about previous unreal work that i did using blueprints and a good way to show my c++ skills was to create a specific purpose system and creating a plugin from it so its easy to implement in other projects, this would also be a great portfolio peace. For one of my personal portfolio projects I created a Manually reloadable gun system using blueprints recreating this using c++ would be good practice and a solid peace of work. He also gave me a few pointers like recreating all the logic in c++ but also combine it with blueprints for customization. 

## Daniel Valente de Macedo Week 7
We discussed the [[System design chart (pre-development).canvas]] looking at the components and actors, Daniel was enthusiastic about it. It looked good. He says a good research and idea of how the system might look helps start the start of the development process. We also looked at the blueprint version of the system. I also asked about already existing systems on fab and he told me that it can help to read their documentation if available as seeing how others solved the problem can help you.

## Daniel Valente de Macedo Week 11
**Synopsis:**
We discussed the technical implementation of the VR Gun System plugin, focusing on modularity, C++ interfaces, and content organization. Daniel recommended providing example scenes with various gun types to demonstrate the system's flexibility. We reviewed the C++ GrabComponent, the transition to StateTree for weapon logic, and the use of DataAssets for configuration. Daniel advised moving away from string-based mesh matching in favor of gameplay tags and explained how to structure the plugin's content folder so assets are bundled with the code.

**Key Technical Points:**
- **Interface-Based Interaction:** Communication between hands and objects via `BPI_Interactor` and gameplay tags (e.g., `Trigger`, `PrimaryAction`) ensures a decoupled architecture.
- **Custom GrabComponent:** A C++ implementation that handles physics (disabling simulation on grab) and manual velocity calculation for realistic throwing, improving on the standard Unreal template.
- **Weapon Flow with StateTree:** Managing complex weapon states (Idle, Fire, Eject, Chamber) using StateTrees for a highly modular and flexible logic system.
- **Socket Snapping:** Implementing "Snap to Socket" to ensure correct visual alignment of the gun in the hand.
- **Plugin Content Management:** Storing meshes and materials in the plugin's `Content` folder ensures they are correctly packaged and accessible to end-users when the plugin is shared.

**Transcription Highlights:**
- **Bo:** "I'm struggling with what I want the plugin to do vs what the user has the freedom to do."
- **Daniel:** "You should at least have some examples... showing 3-4 types of guns... to show it's flexible enough."
- **Bo:** "I made a grab component for C++... it turns off the simulation... and I calculate the velocity manually because when you hold them... they don't get the velocity on their rigid body."
- **Daniel:** "StateTree is a very powerful system... not just for AI... it's very generic and can do way more."
- **Daniel:** "If you share your plugin, that [Content] folder will go together... you can share assets like meshes, animations, or maps as a sample project."

