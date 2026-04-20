# Threshold Protocol - Game Design Document

^a1c452

**Date:** 03 February 2026  
**Genre:** First-Person Survival Horror  
**Engine:** Unreal Engine (C++ Focus)  
**Timeline:** 17 Weeks  

---

## 1. High Concept
A first-person survival horror game set in a "thick" atmospheric forest that serves as a containment site for an Altered World Event (AWE). The player, a crash survivor, must escape a relentless, shapeless entity that hunts them through the dark. The experience focuses on **Systemic Horror**—using dynamic audio and "Herd and Hunt" AI to create tension without relying on scripted sequences or high-fidelity assets.

**Inspirations:** *Alan Wake* (Atmosphere/Light), *Control* (Narrative Tone/FBC), *Alien: Isolation* (AI).

---

## 2. Core Pillars

### A. Systemic Horror (The Invisible Enemy)
Fear is derived from systems, not jump scares. A "Game Director" manages the pacing, while the monster uses "Quantum Stalker" logic to exist perpetually in the player's periphery.

### B. Auditory Atmospherics
The environment is the primary antagonist. A C++ Soundscape Manager dynamically alters the audio mix (low-pass filters, silence, distortion) based on the monster's proximity and the player's "Otherness" exposure.

### C. Body Awareness
Movement feels physical and desperate. The character controller emphasizes weight, inertia, and camera movement (bob/tilt) to align the player's mechanical clumsiness with the protagonist's panic.

---

## 3. Core Gameplay Loop

1.  **The Inciting Incident:** Player wakes up after a crash.
2.  **The Weenie:** Drawn to a campfire light in the distance.
3.  **The Reveal:** The figure at the fire is not human. The chase begins.
4.  **The Hunt:** The player must traverse the forest, guided by radio signals (clues), while being herded by the AI.
5.  **The Climax:** Reaching the "White Geometry"—a sensory overload ending where reality dissolves.

---

## 4. Key Systems (Technical Showcase)

[[A. The Quantum Stalker AI (C++ & EQS)]]

[[B. Soundscape Manager]]

[[C. Diegetic Narrative System]]

## 5. Narrative & Aesthetic

*   **Setting:** A dark, dense forest. The "Containment Site."
*   **Tone:** Bureaucratic Supernatural. Cold, clinical government equipment (flickering floodlights, radio towers) contrasting with raw, primal nature.
*   **The Ending (Option 3: White Geometry):**
    *   No standard extraction.
    *   As the player reaches the final coordinates, the world desaturates (Post-Process Material).
    *   Audio cuts to absolute silence ("Dead Zone").
    *   The player walks into a blinding white geometric void—an "Altered World Event" consumption.

---

## 6. Development Roadmap (17 Weeks)

### Phase 1: The Foundation (Weeks 1-6)
*   Implement First-Person Controller (C++ Body Awareness).
*   Build basic Forest Environment (terrain, foliage).
*   Develop base Soundscape Manager.
*   Implement "White Geometry" transition logic.

### Phase 2: The Stalker (Weeks 7-12)
*   Develop AI "Director" Class.
*   Implement EQS "Herd and Hunt" behavior.
*   Code "Quantum Stalker" teleportation logic.
*   Integrate Audio-Visual bush shaking system.

### Phase 3: Narrative & Polish (Weeks 13-17)
*   Record and implement Radio Audio Logs.
*   Add Post-Process Shaders (Static, Desaturation).
*   Refine "Stress" values and pacing.
*   Final Bug Fixes and Optimization.
