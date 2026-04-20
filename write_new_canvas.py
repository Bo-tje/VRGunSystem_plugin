import json

file_path = r'c:\Users\bo\Desktop\git\VRGunSystem_plugin\minor skilled notes\System design\System design chart (in-development).canvas'

nodes = [
    # GROUPS
    {
        "id": "g_core",
        "type": "group",
        "label": "Core Architecture",
        "x": -250, "y": -50, "width": 900, "height": 950,
        "color": "5"
    },
    {
        "id": "g_interaction",
        "type": "group",
        "label": "VR Interaction System",
        "x": 650, "y": -50, "width": 900, "height": 350,
        "color": "2"
    },
    {
        "id": "g_proj_data",
        "type": "group",
        "label": "Ammo Data",
        "x": -700, "y": 300, "width": 400, "height": 600,
        "color": "3"
    },
    
    # NODES
    {
        "id": "n_weapon_base", "type": "text",
        "text": "### Actor: AVRWeaponBase\nBase Weapon actor, container for modular components.\nRoutes VR interaction and manages initialization.\n\n**Variables:** WeaponData, WeaponRoot, PartRoot, StateTreeComponent\n**Functions:** InitializeWeapon(), ApplyWeaponDataVisuals(), PrimaryAction(), PullTrigger()",
        "x": 0, "y": 0, "width": 400, "height": 260
    },
    {
        "id": "n_chamber", "type": "text",
        "text": "### Component: UVRChamberComponent\nManages the round currently in the firing position.\n\n**Variables:** CurrentChamberState, LoadedProjectile\n**Functions:** TryLoad(), TryEject(), TryGiveBullet(), IsRoundReady(), IsEmpty()",
        "x": -200, "y": 350, "width": 350, "height": 240
    },
    {
        "id": "n_fire", "type": "text",
        "text": "### Component: UVRFireComponent\nHandles the actual firing logic (hitscan / projectile spawning).\n\n**Variables:** MuzzleSocketName, FireHapticScale, DryFireHapticScale\n**Functions:** HandleFiring(), HandleDryFire(), GetMuzzleTransform()",
        "x": 250, "y": 350, "width": 350, "height": 240
    },
    {
        "id": "n_state_tree", "type": "text",
        "text": "### Logic: VR Weapon State Tree\nDrives weapon states dynamically via schema context.\n\n**Evaluators:**\n- IsTriggerPulled\n- HasRoundReady\n- ChamberStateTag\n\n**Tasks:**\n- FSTTask_FireWeapon\n- FSTTask_EjectRound\n- FSTTask_ChamberRound",
        "x": 0, "y": 650, "width": 400, "height": 220
    },
    {
        "id": "n_weapon_data", "type": "text",
        "text": "### Data Asset: UVRWeaponData\nDefines the configuration for a weapon.\n\n**Variables:**\nFireRate, UseHitscan, RecoilAmount, CompatibleMagazinesTag\nWeaponParts, DefaultProjectile\nFireSound, MuzzleFlash",
        "x": 0, "y": -400, "width": 400, "height": 240
    },
    {
        "id": "n_proj_data", "type": "text",
        "text": "### Data Asset: UProjectileData\nProperties of a round.\n\n**Variables:**\nDamage, HitscanDamage\nInitialSpeed, GravityScale, ProjectileClass\nHitscanRange\nLiveRoundMesh, SpentCasingMesh",
        "x": -675, "y": 350, "width": 350, "height": 250
    },
    {
        "id": "n_mag_data", "type": "text",
        "text": "### Data Asset: UMagazineData\nProperties of a magazine.\n\n**Variables:**\nMaxAmmo, MagazineType, MagazineMesh",
        "x": -675, "y": 650, "width": 350, "height": 180
    },
    {
        "id": "n_grab_comp", "type": "text",
        "text": "### Component: UVRGrabComponent\nPlaced on grabbable actors.\n\n**Variables:**\nIsHeld, UseSocketSnap, GrabSocketName\nThrowMultiplier, GrabHapticEffect\n**Functions:**\nTryGrab(), TryRelease(), IsHeld(), GetHoldingHand()",
        "x": 700, "y": 0, "width": 350, "height": 260
    },
    {
        "id": "n_interactor", "type": "text",
        "text": "### Component: UVRInteractor\nPlaced on VR hands.\n\n**Variables:**\nHandSide, ActiveGrabComponent, HoverHapticEffect, HoverTarget\n**Functions:**\nIntendGrab(), IntendRelease(), IntendActionStart(), IntendActionStop(), PlayHapticFeedback()",
        "x": 1150, "y": 0, "width": 350, "height": 260
    },
    {
        "id": "n_i_weapon", "type": "text",
        "text": "### Interface: IVRWeaponInterface\n**Functions:**\nPullTrigger(), PrimaryAction(), SecondaryAction(), IsTriggerPulled()",
        "x": 700, "y": 400, "width": 350, "height": 160
    },
    {
        "id": "n_i_weapon_comp", "type": "text",
        "text": "### Interface: IVRWeaponComponentInterface\n**Functions:**\nInitializeComponent(UVRWeaponData* InData)",
        "x": 700, "y": 600, "width": 350, "height": 140
    },
    {
        "id": "n_i_round", "type": "text",
        "text": "### Interface: IVRRoundProvider\n**Functions:**\nGetRound(UProjectileData*& OutRound)",
        "x": -675, "y": 150, "width": 350, "height": 140
    },
    {
        "id": "n_i_interact", "type": "text",
        "text": "### Interface: IVRInteractableInterface\n**Functions:**\nStartAction(), StopAction(), OnHoverStart(), OnHoverEnd()",
        "x": 700, "y": -250, "width": 350, "height": 160
    },
    {
        "id": "n_i_interactor", "type": "text",
        "text": "### Interface: IVRInteractorInterface\n**Functions:**\nGetProvidingPlayerController()",
        "x": 1150, "y": -250, "width": 350, "height": 140
    }
]

edges = [
    # Asset relations
    {"id": "e_weapdata_base", "fromNode": "n_weapon_data", "fromSide": "bottom", "toNode": "n_weapon_base", "toSide": "top"},
    {"id": "e_proj_chamber", "fromNode": "n_proj_data", "fromSide": "right", "toNode": "n_chamber", "toSide": "left"},
    
    # Core architecture
    {"id": "e_base_chamber", "fromNode": "n_weapon_base", "fromSide": "bottom", "toNode": "n_chamber", "toSide": "top", "label": "Owns"},
    {"id": "e_base_fire", "fromNode": "n_weapon_base", "fromSide": "bottom", "toNode": "n_fire", "toSide": "top", "label": "Owns"},
    {"id": "e_base_statetree", "fromNode": "n_weapon_base", "fromSide": "bottom", "toNode": "n_state_tree", "toSide": "top", "label": "Owns"},
    {"id": "e_tree_chamber", "fromNode": "n_state_tree", "fromSide": "left", "toNode": "n_chamber", "toSide": "bottom", "label": "Tasks/Evaluates"},
    {"id": "e_tree_fire", "fromNode": "n_state_tree", "fromSide": "right", "toNode": "n_fire", "toSide": "bottom", "label": "Task: Fire"},

    # Interaction relations
    {"id": "e_interactor_grab", "fromNode": "n_interactor", "fromSide": "left", "toNode": "n_grab_comp", "toSide": "right", "label": "Interacts with"},
    {"id": "e_grab_base", "fromNode": "n_grab_comp", "fromSide": "left", "toNode": "n_weapon_base", "toSide": "right", "label": "Attached to"},

    # Interfaces
    {"id": "e_base_iweap", "fromNode": "n_weapon_base", "fromSide": "right", "toNode": "n_i_weapon", "toSide": "left", "label": "Implements"},
    {"id": "e_fire_iweap", "fromNode": "n_fire", "fromSide": "right", "toNode": "n_i_weapon", "toSide": "left", "label": "Implements"},
    {"id": "e_chamber_icomp", "fromNode": "n_chamber", "fromSide": "right", "toNode": "n_i_weapon_comp", "toSide": "left", "label": "Implements"},
    {"id": "e_fire_icomp", "fromNode": "n_fire", "fromSide": "right", "toNode": "n_i_weapon_comp", "toSide": "left", "label": "Implements"},
    {"id": "e_chamber_iround", "fromNode": "n_chamber", "fromSide": "left", "toNode": "n_i_round", "toSide": "right", "label": "Implements"},
    {"id": "e_base_iinteract", "fromNode": "n_weapon_base", "fromSide": "right", "toNode": "n_i_interact", "toSide": "left", "label": "Implements"},
    {"id": "e_grab_iinteract", "fromNode": "n_grab_comp", "fromSide": "top", "toNode": "n_i_interact", "toSide": "bottom", "label": "Implements"},
    {"id": "e_interactor_iintctor", "fromNode": "n_interactor", "fromSide": "top", "toNode": "n_i_interactor", "toSide": "bottom", "label": "Implements"}
]

data = {
    "nodes": nodes,
    "edges": edges
}

with open(file_path, 'w', encoding='utf-8') as f:
    json.dump(data, f, indent=4)
