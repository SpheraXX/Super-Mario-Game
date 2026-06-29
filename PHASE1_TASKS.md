# Phase 1: Framework & Engine Initialization

## 📌 Phase 1 Brief
Before we dive into building specific levels, we need to establish the core engine and Object-Oriented framework for our Super Mario game. Phase 1 focuses on setting up the MVC architecture, the SFML graphics window, physics, input, and memory management.

To work efficiently and avoid Git conflicts, the tasks for this phase are strictly divided among the **5 team members**.

---

## 🛠️ GitHub Issues to Create

*Instructions: Copy everything below each title and paste it into a New Issue on GitHub. Assign one team member to each issue.*

### Issue 1: [Core Architect] Game Loop & State Manager
**Assignee:** [Name of Member 1]
**Description:**
Build the foundational application loop and manage the transition between game states.
**Tasks:**
- [ ] Implement `AppEngine` to handle the core Game Loop (Update, Render, Process Input).
- [ ] Apply the **Singleton Pattern** to create a `GameManager` (tracks score, lives, current level).
- [ ] Implement the **State Pattern** with base classes to transition between `MenuState`, `PlayState`, and `GameOverState`.

---

### Issue 2: [Graphics & Audio Engineer] Rendering & Asset Manager
**Assignee:** [Name of Member 2]
**Description:**
Initialize SFML and ensure all textures, fonts, and sound files are loaded efficiently into memory.
**Tasks:**
- [ ] Set up the SFML main application `Window` inside the View layer.
- [ ] Apply the **Singleton Pattern** to build an `AssetManager` that loads `.png` and `.wav` files once and provides pointers to them.
- [ ] Implement generic sprite and text drawing functions.
- [ ] Apply the **Observer Pattern** to handle audio playback when specific events happen (e.g., jumping, eating a mushroom).

---

### Issue 3: [Physics & Collision Dev] Base Entities & World Physics
**Assignee:** [Name of Member 3]
**Description:**
Establish the base classes for all entities and handle the math for movement and collisions.
**Tasks:**
- [ ] Define the abstract `GameObject` class (position, bounding box, virtual `Update()`, `Render()`).
- [ ] Define the abstract `Character` class inheriting from `GameObject`.
- [ ] Implement a custom **AABB (Axis-Aligned Bounding Box)** collision detection system.
- [ ] Implement basic gravity and velocity calculations for characters.

---

### Issue 4: [Data & Level Manager] Factory Pattern & Level Loading
**Assignee:** [Name of Member 4]
**Description:**
Handle parsing level maps from text files and dynamically generating entities on the screen.
**Tasks:**
- [ ] Apply the **Factory Pattern** to create an `EntityFactory` that spawns enemies and items dynamically without tight coupling.
- [ ] Write a `LevelLoader` to read `.txt` files representing our 3 levels and convert them into solid blocks on the screen.
- [ ] Set up basic standard C++ `std::fstream` structures for saving and loading game progress.

---

### Issue 5: [Gameplay & AI Developer] Core Mechanics & Enemy Logic
**Assignee:** [Name of Member 5]
**Description:**
Implement the behaviors of specific characters, input controls, and enemy AI mechanics.
**Tasks:**
- [ ] Implement the `Player` class (handling keyboard inputs for walking and jumping).
- [ ] Handle logic for switching between multiple characters (e.g., Mario and Luigi).
- [ ] Apply the **Strategy Pattern** to implement basic AI behaviors for enemies (e.g., proximity-based chasing vs. simple patrolling).
- [ ] Enforce Git rules for the team: Review PRs, ensure MVC structure is followed, and verify no raw pointers cause memory leaks.

---

## 🤝 Workflow & Peer Review Guidelines

To maintain code quality and structural integrity during Phase 1:
1. **Branching & PRs:** Each member must implement their issue on a separate feature branch (e.g., `feat/game-loop`). Pushing directly to `main` or `dev` is strictly prohibited.
2. **Review Process:** Before any branch can be merged, the PR must be peer-reviewed by the Data Manager and/or Core Architect.
3. **C++ Coding Standards:**
   - Always use standard `#ifndef` / `#define` / `#endif` header guards. No `#pragma once`.
   - Never use `using namespace std;` in any `.h` files.
   - Enforce the **Rule of Three** for classes managing raw pointers to prevent memory leaks and dangling pointers.
4. **AI Code Safety:** All code designed or assisted by AI models must be validated manually, compiled locally, and tested for performance before approval. Refer to the [AI_Usage_Declaration.md](file:///d:/Private%20Data/D_Downloads/Study/HCMUS/1st%20Year/Semester%203/Programming%20Systems/Mario/AI_Usage_Declaration.md) for individual guidelines.
