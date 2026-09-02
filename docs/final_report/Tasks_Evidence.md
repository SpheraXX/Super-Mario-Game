# Project Tasks & Evidence Log

Below is the detailed breakdown of the tasks performed across the recent development branches (`feat/ui-render`, `feat/pause-assets`, and `feat/ui-integration`). You can use these descriptions, estimated hours, and commit references to fill out your project tracking or evidence forms.

---

## Branch: `feat/ui-render`

### Task 1: Implement Action-Based InputMapper and DIP Coordinate System
- **Task Description**: Decoupled raw hardware keys from gameplay actions by introducing an `IInputMapper` that produces semantic `InputSnapshot` objects. Translated raw window pixels to a logical resolution (Device Independent Pixels - DIP) to support scalable UI layouts on multiple screen resolutions.
- **Hours**: 4
- **Evidence**: Screenshot of Git commit `7cb1d1d` ("refactor: implement action-based InputMapper and apply DIP to UI coordinates").

### Task 2: Enhance UI Components with Visual Scrollbars
- **Task Description**: Developed a interactive visual scrollbar feature for the custom `UIScrollView` component to allow users to smoothly navigate overflowing content lists, complete with handle sizing and dynamic alpha blending logic.
- **Hours**: 3
- **Evidence**: Screenshot of Git commit `226405a` ("feat(ui): add visual scrollbar to UIScrollView and apply DIP coordinate bridging").

---

## Branch: `feat/pause-assets`

### Task 3: Implement Centralized AssetManager for Texture Caching
- **Task Description**: Refactored texture loading across all renderers (`SpritePainter`, `SpriteEntityRenderer`, `BrickBlockRenderer`, `CoinBlockRenderer`) to utilize a unified `AssetManager` singleton. Added support for raw image caching, automatic color-key masking (transparent backgrounds), and hot-reloading.
- **Hours**: 6
- **Evidence**: Screenshots of Git commits `76f4cbb` ("feat(view): implement raw image caching..."), `edf73d2`, or `064fe84`.

### Task 4: Build Dynamic Display Settings and Viewport Scaling System
- **Task Description**: Updated `SettingsManager` to track aspect ratio and V-Sync configuration. Integrated floating-point scaling and letterbox/pillarbox viewport configurations inside `AppEngine` to maintain pixel-perfect rendering across varying window aspect ratios.
- **Hours**: 5
- **Evidence**: Screenshots of Git commits `00aeb9e` ("feat(model): update Settings...") and `5586a61` ("feat(controller): implement display settings logic...").

### Task 5: Implement Observer Pattern for Settings Integration
- **Task Description**: Re-architected `SettingsManager` using the Observer Pattern so subsystems can reactively update when settings change. Developed `OptionsState` menu logic, UI validation checks, and conflict warnings for duplicate keybinds.
- **Hours**: 5
- **Evidence**: Screenshot of Git commit `5927a09` ("feat: integrate graphics settings via Observer Pattern (Phase 1)").

### Task 6: Develop Contextual Menus (`PauseState` & `WarningPopupState`)
- **Task Description**: Created a transparent `PauseState` overlay to halt game execution and allow mid-game settings adjustment. Built `WarningPopupState` (formerly `ConfirmState`) to handle critical user prompts like keybind conflicts, ensuring input focus capture and safe state popping.
- **Hours**: 4
- **Evidence**: Screenshots of Git commits `05165d1` ("feat(controller): implement PauseState...") and `a1be6b0` ("feat: refactor ConfirmState to WarningPopupState...").

---

## Branch: `feat/ui-integration`

### Task 7: Develop UI Visual Effects (LerpAnimator & GLSL Shaders)
- **Task Description**: Introduced `LerpAnimator` for smooth cubic easing transitions in UI components. Built a `MetalShineEffect` utilizing custom GLSL fragment shaders (`metal_shine.frag`) to add dynamic, sweeping specular highlights to the main title logos.
- **Hours**: 4
- **Evidence**: Screenshot of Git commit `dd0360d` ("feat(effect): add LerpAnimator and MetalShineEffect GLSL shader").

### Task 8: Overhaul Game Menus (MainMenu, Intro, and Credits)
- **Task Description**: Built the `IntroState` and `CreditsState` with looping scroll mechanics driven by external JSON data. Integrated background (BGA) dimming, slider animations, and standardized `UITheme` color palettes and layouts across all menus.
- **Hours**: 6
- **Evidence**: Screenshots of Git commits `8c55f02` ("feat(ui): refactor MainMenu..."), `26830f7` ("feat(ui): implement IntroState..."), and `f4ed9b8` ("feat(ui): implement CreditsState...").

### Task 9: Implement World & Level Selection Carousels with Progression Tracking
- **Task Description**: Developed `WorldSelectState` and `LevelSelectState` using a horizontal 3D-card carousel layout. Integrated save data to dynamically lock/unlock worlds and levels based on player progress dependencies defined in `worlds.json`.
- **Hours**: 8
- **Evidence**: Screenshots of Git commits `b15eef8` ("feat(selector): add WorldSelectState..."), `eb13312`, and `52205f9`.

### Task 10: Refactor PlayState Progression Logic & Optimize Render Pipeline
- **Task Description**: Decoupled `PlayState::finishClear` into specific progress and profile tracking helper methods. Optimized `WorldSelectState` by pre-computing unlock flags and caching locked text to eliminate per-frame heap allocations, ensuring a stable 60 FPS UI performance.
- **Hours**: 5
- **Evidence**: Screenshot of Git commit `4b6362c` ("refactor(controller): optimize WorldSelectState rendering...").

### Task 11: Draft Comprehensive Implementation Documentation
- **Task Description**: Authored the full `Section3_Implementation.md` covering the engine's OOP/MVC architectural design, behavioral design patterns (State, Singleton, Command, Observer, Composite), fixed-timestep game loop, custom UI framework, and data-driven progression systems for the final report.
- **Hours**: 6
- **Evidence**: Screenshots of Git commits `b061916` ("docs: add comprehensive Section 3 Implementation chapter...") and `d2b92ff`.
