# Super Mario — System Architecture

> Living overview of the codebase as implemented. The project is a C++17 + SFML 3
> desktop game built with CMake or the checked-in Makefile (winlibs gcc 14.2 toolchain).

---

## 1. Layering

The code is split into three layers plus an entry point. Dependencies point strictly
downward: `Controller -> View -> Model` (the view reads the model, never the reverse).

```
App (src/main.cpp)
  └── Controller  (app flow, game states, input)
        └── View  (rendering, sprite/text handling, HUD)
              └── Model  (pure game logic: entities, physics, levels, world)
```

All game logic lives under `src/Model`; nothing under `View` or `Controller` owns
gameplay state that the model should own.

## 2. Model layer

Pure game logic with no SFML render dependencies (`SFML/Window` is used for input
enum types in `Player`).

### Core

- `model::Vector2` — x/y world position (top-left origin; `+y` is down).
- `model::Entity` — base for everything that lives in a level: position, size,
  velocity, bounding `Hitbox`, alive/active/dying flags, `update(dt)`.
- `model::Character` — `Entity` subclass with gravity, ground state, animations,
  `TileMap` reference for tile collision (set before use).
- `model::CollisionManager` — physics resolution: AABB pass against the `TileMap`
  (swept resolving) and entity-vs-entity pass driven by `CollisionLayer`
  (`Player`/`Environment`/`Enemy`). Emits `BlockHitEvent`s when the player's head
  bumps a block and stomp events when the player lands on an enemy.
- `model::GameManager` — singleton: score, coins, lives, current/next map path,
  level name, clear bonus.
- `model::LevelTimer` — countdown with pause.

### Entities
- `model::Block::*` — `CoinBlock` (one coin + pop animation), `BrickBlock` (bounce):
  both use `startBounce()` and the `onBlockHit` dispatch.
- `model::Enemy::*` — Goomba, Koopa: AI via `updateAI`, tile-wall turn-around,
  stomp handling (squish + despawn timer).
- `model::Player::*` — `Player` (walk/run/sprint, jump with coyote time + jump
  buffering + variable hold height, underwater swim), `Mario`/`Luigi` tunables,
  `PlayerState` state machine (Small/Super/Fire/Star) with damage transitions.
- `model::Level::*` — `Pipe` (2 tiles wide; cap+body), `FlagPole`, `Castle` terminal.

### World & Map
- `model::TileMap` — row-major char grid (`TileWidth`/`TileHeight`), direct rect
  queries, `loadFromLines`.
- `model::Level` — owns the areas: parses `.map` (areas, portals, metadata), keeps
  per-area `TileMap`s, and always appends the terminal area
  (`appendTerminalArea()` reads `assets/maps/flagpolecastle.map`).
- `model::World`/`WorldSet`/`WorldType` — world metadata (overworld/underwater, ...).

## 3. View layer

- `AssetManager` — loads textures from `assets/` once.
- `EntityRendererRegistry` — type-to-renderer dispatch (`registerRenderer<Mario,
  PlayerRenderer>()`); every entity is drawn through its registered renderer.
- `SpriteEntityRenderer` — base for frame-sheet animations (walk cycles, jumps...).
- `TileMapRenderer` — themed tile sheet (blocks.png) per world type.
- `HudRenderer` — score/coins/time/name against the game camera.

## 4. Controller layer

- `AppEngine` — window (`CS202 Super Mario`), fixed logical resolution rendered
  offscreen then upscaled, fixed-timestep main loop with `MaxFrameTime` clamp,
  scale toggle, input pump.
- `StateManager` — state stack with deferred apply (`pushState`/`replaceState`).
- `MenuState` / `PlayState` / `LevelCompleteState` / `GameOverState` — the game flow.

### PlayState (the heart of gameplay)
- Owns the working `TileMap`, entity list, camera, HUD data and clear/warp flows.
- `loadArea(i)` copies the level's area grid into the working map, sets the world
  type, clears the one-way pipe inert set, then `resetLevel()` rebuilds every entity
  from the map symbols (`M` Mario, `E` Goomba, `K` Koopa, `C` coin, `#`/`B` brick,
  `P` pipe, `F` flagpole, `H` castle).
- Pipes: contiguous vertical `'P'` runs become one 2-tile-wide `Pipe` (see below).
- Flagpole touch: `beginLevelClear()` awards `FlagBonus + timeBonus`, then the
  scripted clear plays (slide → walk → castle → `LevelCompleteState`).
- Deaths: `loseLife()`; on game over → `GameOverState`, else `resetLevel()`.

## 5. Warps & one-way pipes

- `Portal` (`include/Model/Level/Level.h`): `sourceColumn` + `destinationColumn` +
  destination area, parsed from `; pipe=col:X,enter:down,to:A:C` lines; the area's
  pipes are registered on map load, `getSourceColumn()` = the pipe's left cell.
- Entry trigger: the player must stand (feet within `8.0f`) on a pipe's cap, overlap
  it horizontally, press/hold Down and the pipe's column must have a portal whose
  destination area is NOT inert.
- One-way rule: after warping, `teleportToPortal()` marks the destination column
  inert for that area visit (cleared at `loadArea`), so a pipe cannot be re-entered
  from the cap it just dropped the player on.
- Transition (frozen while active, `update()` gates at the top):
  `SinkIn -> WashOut -> BlackHold (swap area) -> WashIn -> RiseOut`
  with timings `0.35/0.30/0.45/0.75`s and depths `128/48` world units. During
  `SinkIn`/`RiseOut` the pipe covers Mario (pipes draw in a second pass).
- Every area visit re-spawns everything from the loaded area grid.

## 6. Flow / maintenance notes

- The level map is the single source of truth for where entities spawn: marks map
  1:1 to positions (row 0 = bottom). `assets/maps/flagpolecastle.map` is appended to
  every level and gate/finish-in lines are auto-detected; the terminal flag/castle
  are always present so `LevelComplete` is reachable from any area.
- `mm`/`cmake` builds from `build.out`; the smoke suite (model-only, headless) runs
  in `smoke.cpp` at dev time (101 checks) — see `.opencode/smoke/`.

## 7. Known gaps (deliberate scope)
- No worlds beyond the level map file, no checkpoints; fireballs/star timers exist
  in states but no projectile rendering yet.
- The renderer is render-entity batched per type with a registry; not SFML
  tile-batched.