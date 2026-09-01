# AI Usage Declaration (Super Mario Project)

*This document outlines how AI tools are utilized by each member during the development of the Super Mario C++ Final Project to ensure transparency and maintain high code quality standards.*

---

## Member 1: Core Architect (Game Loop & State Manager)
- **AI Tool**: Gemini / Copilot
- **Assigned Tasks**: Game loop implementation (`AppEngine`), GameManager (Singleton), State pattern setup.
- **AI Utilization Guidelines**:
  - Research efficient application loop timing to handle variable frame rates.
  - Review implementation of State design pattern templates in C++.
  - Double-verify that state changes do not leak memory when states are allocated on the heap.

## Member 2: Graphics & Audio Engineer (Rendering & Asset Manager)
- **AI Tool**: Gemini / ChatGPT
- **Assigned Tasks**: SFML window setup, AssetManager (Singleton), Observer pattern for audio events.
- **AI Utilization Guidelines**:
  - Search SFML documentation for loading textures/sound buffers dynamically.
  - Query optimized caching structures for asset loading (minimizing disk read overhead).
  - Draft basic observer registry patterns.

## Member 3: Physics & Collision Dev (Base Entities & World Physics)
- **AI Tool**: Claude / Gemini
- **Assigned Tasks**: `GameObject` hierarchy, AABB collision system, physics/gravity calculations.
- **AI Utilization Guidelines**:
  - Solve specific math logic for swept AABB (Axis-Aligned Bounding Box) to prevent tunneling.
  - Optimize velocity accumulation formulas.
  - Request edge cases for platformer collisions (e.g., corner catching, sliding).

## Member 4: Data & Level Manager (Factory Pattern & Level Loading)
- **AI Tool**: Copilot / Gemini
- **Assigned Tasks**: EntityFactory, `LevelLoader` (file parser), game progress serialization (Save/Load).
- **AI Utilization Guidelines**:
  - Code generation for binary/text serialization structures in C++.
  - Implement Factory pattern without creating circular dependencies between entities.
  - Write test files generators for level maps.

## Member 5: Gameplay & AI Developer (Core Mechanics & Enemy Logic)
- **AI Tool**: Gemini / Cursor
- **Assigned Tasks**: Player controller, character switching, Goomba/Koopa State Machine (Strategy pattern).
- **AI Utilization Guidelines**:
  - Query state transition logic for entity AI.
  - Analyze keyboard input buffering strategies for smooth gameplay mechanics.

---

## Verification & Joint Responsibility
All team members agree that **all code generated, optimized, or reviewed by AI tools must be manually verified, compiled, and peer-reviewed** in Pull Requests before merging into the `main` or `dev` branches. The team assumes full responsibility for the stability, correctness, and architecture of the final application.
