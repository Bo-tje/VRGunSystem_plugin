### A. The "Quantum Stalker" AI (C++ & EQS)
*   **Herding Logic:** The AI does not just chase; it uses the Environment Query System (EQS) to find flank locations, pushing the player toward narrative markers.
*   **Relocation Manager:** If the distance exceeds a `MaxTensionRange` (e.g., 50m) and Line of Sight is broken, the monster seamlessly "teleports" to a calculated intercept point ahead of the player.
*   **Masking:** Teleportation is masked by diegetic effects—radio static spikes or distant "thuds" to simulate rapid movement.



3 systems

knowing where player is and moving towards him

knowing where current poi is and steering towards it using the before explained teleportation system

when in a certain radius of the player switch to line of sight 

why this system:
the monster wont roam endlessly trough the forest but wil hunt the player. when getting close it goes to line of sight not knowing where the player is to make it feel more like a hunt instead of a NPC homing in. to try and herd the player the monster will approach from angles towards the next poi