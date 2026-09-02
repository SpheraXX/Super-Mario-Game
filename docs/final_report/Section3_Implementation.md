# 3 Implementation

## 3.1 Overview
The architectural foundation of the *Super Mario Bros.* engine rigorously adheres to Object-Oriented Programming (OOP) principles and the Model-View-Controller (MVC) paradigm. The primary objective of this architecture is to establish a modular, scalable, and loosely coupled system capable of handling complex 2D platforming physics, dynamic UI rendering, and persistent session management.

- **Abstraction** is enforced via abstract base classes and pure virtual interfaces. For example, `GameState` abstracts the lifecycle of individual application screens, `UIElement` abstracts graphical user interface components, and `IInputMapper` isolates logical input intent from hardware-specific key codes. This shields the core game logic from low-level multimedia library (SFML) dependencies.
- **Encapsulation** protects the integrity of sensitive subsystems. The `SettingsManager` and `SaveManager` restrict direct data mutation by encapsulating file I/O operations and providing thread-safe, sanitized mutator methods that automatically trigger persistence mechanisms.
- **Polymorphism** serves as the backbone for runtime behavior execution. The engine relies heavily on dynamic dispatch—whether in `Entity::update(deltaTime)` for physics resolution across varied actors, or in virtual event handlers (`handleEvent`) overriding default behaviors in specialized UI components like `UISlider` or `UIScrollView`.
- **Inheritance** establishes strict logical taxonomies. This is most prominent in the Entity hierarchy (where `Character` and `Block` extend `Entity`, and specific actors like `Mario` or `Goomba` extend `Character`), as well as in the `UIContainer` tree structure for composing complex interface layouts.

## 3.2 Design patterns

### 3.2.1 State Pattern
The State design pattern governs the macroscopic flow of the application via the `StateManager` and `GameState` interface. Each discrete screen—such as `MenuState`, `PlayState`, `IntroState`, `CreditsState`, `OptionsState`, `WarningPopupState`, and `LevelSelectState`—is encapsulated as an independent state object. This pattern is also utilized internally on a microscopic level via the `PlayerState` interface. `PlayerState` allows Mario to dynamically shift his capabilities and visual representations between `SmallState`, `SuperState`, `FireState`, and `StarState` without executing complex conditional logic trees inside the main `Player` class.

### 3.2.2 Singleton Pattern
System-wide managers that mandate a singular, universally accessible instance are implemented as Singletons. 
- `GameManager` acts as the persistent ledger for scoring, life counts, and current map paths across state transitions.
- `AssetManager` provides centralized caching for textures, fonts, and audio buffers, preventing redundant heap allocations and redundant disk I/O.
- `SaveManager` manages the serialization and deserialization of the JSON-backed persistence file. 
These singletons strictly delete their copy constructors and assignment operators to guarantee singular instantiation.

### 3.2.3 Command Pattern
To decouple input triggers from execution logic, the Command pattern is heavily integrated into the newly implemented custom UI framework. Interactive components, such as `UIButton`, hold encapsulated `std::function<void()>` callbacks. This enables menu constructors to bind deferred operations—such as pushing a new `WarningPopupState`, invoking a save routine, or popping the stack—without hard-coupling the visual button widget to the underlying state machine or game controller.

### 3.2.4 Observer Pattern
The Observer pattern is leveraged to create a reactive configuration architecture within the `SettingsManager`. The manager acts as the subject, allowing other subsystems to register callback functions (`std::function<void()>`). When a configuration variable is mutated (e.g., modifying the master volume, toggling V-Sync, or rebinding the jump key in the `OptionsState`), the manager broadcasts these diffs to all subscribed listeners. This allows the `AudioManager` to live-update volume outputs and the `AppEngine` to instantly reconfigure the rendering window without requiring a system restart.

### 3.2.5 Composite Pattern
The custom Graphical User Interface (GUI) framework is structured upon the Composite pattern. The `UIContainer` class implements the standard `UIElement` interface while simultaneously managing an internal `std::vector<std::unique_ptr<UIElement>>` of child nodes. This recursive tree structure facilitates hierarchical coordinate translations (parent-relative positioning), cascading event propagation, and bounding-box layout computations (e.g., auto-centering and vertical stacking).

## 3.3 Program loop

### 3.3.1 Fixed-timestep game loop
The `AppEngine` coordinates execution using a highly deterministic, fixed-timestep loop (`1/60s`). Instead of passing a variable delta-time directly into the physics engine, it utilizes an accumulator model. Elapsed frame time is accumulated, and the engine consumes this time in discrete, uniform slices (`TimeStep`). To prevent the "spiral of death" (where processing a frame takes longer than the simulated time, causing infinite accumulation), the engine imposes a `MaxFrameTime` cap (0.25s). This guarantees that physics simulations, collision penetrations, and `LerpAnimator` UI effects perform identically regardless of the host machine's hardware refresh rate.

### 3.3.2 State stack and deferred transitions
Transitions between scenes are strictly deferred. When a state requests a change (via `pushState`, `popState`, or `replaceState`), the request is queued and only enacted via `applyPending()` at the absolute end of the frame cycle. This guarantees that container iterators are not invalidated mid-update. 
Furthermore, the stack architecture supports transparent overlays utilizing the `isTransparent()` virtual boolean. When a state like `PauseState` or `WarningPopupState` is active, the `StateManager` halts `update()` calls for lower states but continues to call their `render()` functions. This creates a seamless graphical overlay over frozen gameplay or underlying menu screens.

## 3.4 Global structures

### 3.4.1 GameManager singleton state
The `GameManager` acts as the definitive source of truth for runtime session metrics. It abstracts cross-level statistics (score, total coins, remaining lives) and manages the active map routing pipeline (`currentMapPath`, `nextMapPath`). This ensures absolute data consistency when transitioning between the overarching `WorldSelectState` and the granular `PlayState`.

### 3.4.2 Audio & Settings Management
Global environmental and configuration data are maintained by the `AudioManager` and `SettingsManager`. 
- The `AudioManager` parses an `audio_meta.json` file to map string keys to file paths, providing non-blocking, throttled SFX playback via `std::list<sf::Sound>`. It prunes stopped sounds automatically to prevent audio channel saturation and streams heavy background music (BGM) directly from the disk.
- The `SettingsManager` utilizes the `nlohmann::json` library to serialize display resolution, sound volumes, and control bindings into a `settings.json` file, granting persistent cross-session configurations.

### 3.4.3 Save & Persistence Management
The `SaveManager` provides a robust serialization pipeline for the `GameSaveData` structure. It archives not only macroscopic metrics but precise microscopic snapshots of the `PlayState`—including exact `Entity` coordinates, `Enemy` velocity vectors, `Player` power-up forms, and the active/broken status of specific `CoinBlock`s and `BrickBlock`s. This comprehensive snapshot allows features like "Continue Game" or `LevelSelectState` to rebuild exact gameplay environments flawlessly.

### 3.4.4 Profile & Multi-Slot Save Management Framework
To support multi-user progress tracking and independent career saves, the architecture introduces a dual-layer profile persistence subsystem via `ProfileManager` and slot-bound `SaveManager` routing.
- **`ProfileManager`**: Acts as a high-level manager encapsulating up to four user profile slots (`PRO MARIO`, `MID LUIGI`, `NEW PEACH`, `EMPTY`). It tracks active profile selections (`active_profile_index`), cumulative total scores, and total completed levels across sessions in a centralized `profiles.json` ledger.
- **Slot-Based Save Routing**: `SaveManager::getActiveSavePath()` dynamically binds persistence reads and writes to dedicated slot files (`save_slot_0.json` through `save_slot_3.json`). This guarantees complete isolation between profile states while preserving uniform schema serialization.
- **Stat Synchronization**: Upon clearing a stage in `PlayState::finishClear()`, high-score deltas and first-time level completion flags are computed and synced back to `ProfileManager`. This updates global ranking standings dynamically without disk re-read overhead.

## 3.5 Utility objects

### 3.5.1 Tile and Entity collision resolution
The `CollisionManager` performs continuous swept-Axis-Aligned Bounding Box (AABB) intersection testing. To maintain a 60 FPS target with high entity counts, it utilizes broad-phase spatial culling to isolate relevant grid sectors surrounding an entity before applying rigorous narrow-phase penetration depth resolution. The engine incorporates precise epsilon values to prevent floating-point rounding errors (which cause jittering or erroneous ground-state toggling), ensuring rigid body integrity against solid map tiles and facilitating smooth kinetic bounce physics.

## 3.6 Structural objects

### 3.6.1 Map format parsing and area grids
The custom level loader translates ASCII text-based `.map` files into a mathematical 2D spatial grid. The parser reads lines in a bottom-up sequence (`y = (15 - row) * 16`), handling multi-area headers, binding warp portals (`; pipe=`), and instantiating interactive moving platforms (`; slider=`) dynamically at load time.

### 3.6.2 TileMapRenderer atlas indexing
The `TileMapRenderer` caches level geometry by mapping ASCII characters to an `std::unordered_map<char, sf::IntRect>` indexing a primary texture atlas. It avoids drawing empty or out-of-bounds terrain by optimizing render calls, restricting internal `for`-loops strictly to the grid bounds visible to the current camera viewport.

### 3.6.3 Custom UI Framework & DIP Resolution
A completely bespoke UI framework was engineered to manage complex menu layouts. It abstracts hardware resolutions using Device Independent Pixels (DIP), scaling interactive zones proportionally via a `windowToLogical()` mathematical coordinate translation. Advanced components like `UIScrollView` compute `sf::View` clipping rectangles to mask overflowing content, while `UISlider` and `UICycleButton` calculate absolute positioning automatically using vertical auto-layout padding and pixel snapping.

### 3.6.4 Dynamic Data-Driven World & Level Progression Framework
To replace hardcoded progression limits with an extensible structural pipeline, level sequence constraints and world metadata are entirely externalized into `assets/data/worlds.json` and managed at runtime via `WorldManager`.
- **Dependency-Based Level Unlocking**: Each stage record contains an explicit `unlock_requires` attribute (e.g., `"1-2"` requires `"1-1"`). Upon stage completion in `PlayState::updateProgressAndUnlocks()`, the engine evaluates these dependency rules against `GameSaveData::level_progress`, marking dependent levels as `"available"` and appending parent world IDs to `unlocked_worlds`.
- **Multi-Area World Classification**: Levels composed of multiple sub-areas (such as `Overworld` entrance transitioning into `Underground` or `Underwater` sub-maps) compute primary environmental dominance based on tile column spans. This allows `PlayState` to perform real-time area `WorldType` change detection during pipe transitions and dynamically switch background music tracks without violating MVC boundary rules.

### 3.6.5 World Select Carousel & Cached Render Pipeline
The `WorldSelectState` implements a high-performance 3D-card carousel interface:
- **Interpolated Carousel Motion**: Smooth horizontal sliding is computed using a cubic easing `LerpAnimator` (`OutCubic`), dynamically centering focused world banners while scaling and dimming adjacent cards based on normalized screen distance.
- **Pre-computed State & Lock Caching**: To eliminate per-frame heap allocations and text layout calculations during rendering, `WorldSelectState` pre-computes unlock status vectors (`m_isUnlocked`) during `onEnter()`. Locked visual indicators (dimmed shader tinting and `LOCKED` overlay text) are rendered with zero redundant string-to-glyph generation overhead.

## 3.7 Game entities and Mechanics

### 3.7.1 Player physics, movement, and controls
The `Player` model computes horizontal acceleration curves, friction/drag coefficients, and gravity parabolas. Hardware keyboard inputs are entirely decoupled from physical execution via an `IInputMapper` interface. The mapper translates raw key codes (configured dynamically in `settings.json`) into semantic `InputSnapshot` flags (`Jump`, `Run`, `MoveLeft`, `MoveRight`). This enforces strict MVC separation and enables seamless runtime key rebindings from the `OptionsState`.

### 3.7.2 PlayerState transformations
Damage execution and power-up collections dictate dynamic state transformations between `SmallState`, `SuperState`, `FireState`, and `StarState`. The engine manages temporary `damageCooldown` invulnerability periods during transformations, temporarily disabling the collision mask and toggling visual transparency flashing to provide player feedback without compromising the entity update loop. The `StarState` acts as a specialized decorator, wrapping the underlying state while injecting high-speed parameters and lethal contact rules.

### 3.7.3 EnemyFactory instantiation and AI behaviors
Enemies are strictly instantiated parametrically via the `EnemyFactory` based on digit markers (`0-8`) placed in the `.map` grids. AI behaviors are entirely encapsulated within their respective subclass `update()` routines. Behaviors range from the basic fixed-speed patrolling and ledge-falling of a `Goomba`, to the complex tracking algorithms of `Lakitu` (which continuously queries `World::getPlayer()` to hover above the character), and the arc-based projectile trajectory calculations of `HammerBro`.

### 3.7.4 Blocks, Items, and Interactive triggers
Interactive terrain elements derive from the polymorphic `Block` class. `CoinBlock` and `BrickBlock` actively monitor the normal vectors of incoming collisions. An upward normal collision initiated by a super-state player triggers kinetic bounce states and spawns dependent item entities (`Mushroom`, `FireFlower`, `Starman`). Additionally, non-solid trigger regions, such as the `LevelGoal` (pole) or the `ChainTrigger` (axe), perform purely spatial intersections with the player to gracefully advance the `PlayState` cinematic sequence and conclude the level.
