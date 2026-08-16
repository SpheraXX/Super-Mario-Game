# Merge Analysis: `tmp` ↔ `feat/adding-new-entities-state-movements`

Status: **analysis only — no merge has been performed.**
Generated 2026-08-11 against `tmp` = `39bcf13` (k.2.6), `feat` = `7e9ae6e`.

---

## 1. Situation

| | |
|---|---|
| Common ancestor | `c74283c` — *"feat: add new entities and player state movements"* (2026-08-05) |
| `tmp` since ancestor | 14 commits, 101 files, **+4,682 / −1,444** |
| `feat` since ancestor | 7 commits, 82 files, **+3,087 / −323** |
| Files modified by **both** | **31** |
| Files that **conflict** | **26** |
| Conflict hunks | **65** (~1,800 conflicted lines) |

`tmp` already merged an earlier `feat` at `37c966e`, then did 13 commits (`k.2.0`→`k.2.6`) of
architectural refactoring. `feat` continued building on the pre-refactor architecture. That is
the entire source of the divergence.

### Conflict volume by file

| Hunks | Conflicted / total lines | File |
|---:|---:|---|
| 8 | 409 / 495 | `src/Controller/PlayState.cpp` |
| 8 | 362 / 653 | `src/Model/Core/CollisionManager.cpp` |
| 4 | 60 / 92 | `include/Controller/PlayState.h` |
| 4 | 38 / 195 | `src/Model/Character.cpp` |
| 3 | 125 / 139 | `src/View/HudRenderer.cpp` |
| 3 | 118 / 437 | `src/Model/Player/Player.cpp` |
| 3 | 104 / 190 | `src/View/Map/TileMapRenderer.cpp` |
| 3 | 59 / 197 | `src/Model/Map/TileMap.cpp` |
| 3 | 50 / 117 | `include/Model/Map/TileMap.h` |
| 3 | 34 / 117 | `include/Model/Core/GameManager.h` |
| 3 | 25 / 126 | `include/Model/Character.h` |
| 2 | 100 / 141 | `src/Model/Block/CoinBlock.cpp` |
| 2 | 73 / 162 | `include/Model/Player/Player.h` |
| 2 | 43 / 124 | `src/Model/Enemy/Enemy.cpp` |
| 2 | 22 / 88 | `include/Model/Entity.h` |
| 2 | 22 / 55 | `include/Model/Block/CoinBlock.h` |
| 1 | 53 / 188 | `src/View/Player/PlayerRenderer.cpp` |
| 1 | 24 / 40 | `src/View/Enemy/GoombaRenderer.cpp` |
| 1 | 20 / 36 | `src/View/Enemy/KoopaRenderer.cpp` |
| 1 | 18 / 28 | `assets/maps/debug.map` |
| 1 | 16 / 47 | `include/View/HudRenderer.h` |
| 1 | 15 / 39 | `include/View/Player/PlayerRenderer.h` |
| 1 | 8 / 155 | `src/Model/Core/GameManager.cpp` |
| 1 | 7 / 41 | `include/Model/Player/Mario.h` |
| 1 | 7 / 30 | `include/Model/Player/Luigi.h` |
| 1 | 5 / 47 | `src/Model/Entity.cpp` |

---

## 2. What each branch uniquely brings

Neither branch is a subset of the other. Both added ~48 new files.

**`feat` only — content and runtime systems (48 new files)**
- 5 enemies: `Bowser`, `Lakitu`, `HammerBro`, `PiranhaPlant`, `Spiny`, plus `EnemyFactory`
- 5 projectiles: `Fireball`, `MarioFireball`, `Hammer`, `SpinyEgg`, `Projectile` base
- 5 items: `Mushroom`, `FireFlower`, `Starman`, `Coin`, `Item` base
- View: `AtlasFrameRenderer`, `EnemyAtlas`, `ItemAtlas`, `ItemFrameRenderer`, `FireballRenderer`
- Docs: `CORE_ENGINE_REPORT.md`, `ENEMIES.md`, `class_diagram_raw.md`

**`tmp` only — level structure and view infrastructure (48 new files)**
- `Level` (multi-area), `Pipe` + `PortalSystem`, `FlagPole` + castle painting
- `LevelScene`, `LevelClearSequence`, `LevelCompletion`, `LevelCompleteState`, `LevelTimer`
- `WorldSet`, `WorldType`, `BrickBlock`
- View: `AssetManager`, `SpritePainter`, `RenderContext`, `TextUtils`, `HudData`

---

## 3. Conflicts that MUST be resolved

Grouped by the kind of work each requires.

---

### TIER 1 — Blocking. Resolve before anything else.

#### C-1. Duplicate `model::World` class ⚠️ SILENT — git does NOT flag this

The two branches have a class named `World` in namespace `model` at **different paths**.
Because the paths differ, git merges both in with **zero conflict markers**, producing two
competing definitions of `model::World`. The build breaks with nothing pointing at the cause.

| | `tmp` — `include/Model/World/World.h` | `feat` — `include/Model/Core/World.h` |
|---|---|---|
| Concept | Immutable **theme + physics descriptor** | Abstract **service interface** |
| Members | `getBackgroundColor()`, `getTilesetPath()`, `getGravityScale()`, `getMaxFallScale()`, `getHorizontalDrag()` | `spawn(unique_ptr<Entity>)`, `getPlayer()` |
| Kind | concrete value object, built by `WorldSet` | pure virtual, implemented by `PlayState` |
| Constructed by | `WorldSet::forType()` registry | n/a (interface) |

**These are unrelated abstractions that collided on a name. Both are needed. Neither dies.**

**Resolution:** rename one. Recommended — rename `tmp`'s to `WorldTheme` (its own header comment
calls it "immutable descriptors"), keep `feat`'s as `World`. Update `WorldSet::forType()`,
`Character::setWorld/worldPtr`, and the renderer's theme lookup.

**Cost:** low, mechanical. **Do this first** — it is independent of every other decision.

---

### TIER 2 — Genuine either/or. Requires a team decision.

#### C-2. Where `velocity` / `isGrounded` live

> **Correction to an earlier draft of this document.** A previous version claimed this conflict
> "touches every one of `feat`'s 16 new projectile/item files (they all assume `Entity::velocity`)."
> **That is false.** Verified against the source: every moving class in `feat` already derives from
> `Character`. The conflict is confined to roughly 16 call sites in a single file. The cost of this
> decision is far lower than that draft implied.

| | `tmp` | `feat` |
|---|---|---|
| `velocity` | `Character` (protected) | `Entity` (public) + `getVelocity/setVelocity` |
| `isGrounded` | `Character` | `Entity` |
| `isDormant` | — | `Entity` |
| Stated rationale | "Nothing in this interface implies life, motion or input, so e.g. a FlagPole can never be asked for a velocity" | uniform treatment in the collision loop |

##### The hierarchies are NOT in conflict

Both branches agree on class structure. `feat`'s hierarchy, verified:

```
Character  : Entity        Enemy : Character       Item       : Character
Player     : Character     NPC   : Character       Projectile : Character
Block      : Entity        (Goomba/Koopa/Bowser/Lakitu/HammerBro/PiranhaPlant/Spiny : Enemy)
                           (Mushroom/FireFlower/Starman/Coin : Item)
                           (Fireball/MarioFireball/Hammer/SpinyEgg : Projectile)
```

The **only** direct `Entity` subclass in `feat` is `Block`, which is static and never moves.
`feat`'s own `Projectile.h` says it outright: *"Derives from Character to reuse gravity and
velocity integration."* Same for `Item.h`.

**Consequence: every content class in `feat` compiles unchanged under either design.**

##### The real question

Both branches store the level as one heterogeneous container:

```cpp
std::vector<Entity*> entities;   // Blocks, Pipes, FlagPole, Player, Enemies, Items, Projectiles
```

`CollisionManager` walks that list and must touch `velocity` and `isGrounded`. The entire conflict
is **how it reaches them from an `Entity*`.**

```cpp
// feat — no cast needed; the fields are on Entity
// CollisionManager.cpp:115, 128, 156
entity->isGrounded = false;
Vector2 vel = entity->getVelocity();

// tmp — must downcast; the fields are on Character
// CollisionManager.cpp:88-91
auto* character = dynamic_cast<Character*>(entity);
if (!character || character->isDying()) continue;
character->isGrounded = false;
processTileCollisions(*character, deltaTime);
```

Signature divergence that follows:

```
tmp   void processTileCollisions(Character& entity, float deltaTime);
      void pushOutOfBlock(Character& mover, const Entity& blocker, CollisionType side);

feat  void processTileCollisions(Entity* entity, float deltaTime);
      void pushOutOfBlock(Entity& mover, const Entity& blocker, CollisionType side);
```

This is the main driver of the 8-hunk `CollisionManager.cpp` conflict.

##### Blast radius — measured, not estimated

Every site in `feat` reaching these through an `Entity`:

| Member | File and lines | Count |
|---|---|---:|
| `isGrounded` | `CollisionManager.cpp` 115, 156, 365 | 3 |
| `getVelocity` / `setVelocity` | `CollisionManager.cpp` 39, 60, 77, 128, 236, 330, 364, 369, 375, 380 | 10 |
| helper signatures | `isStompFromAbove`, `hasStableTopContact`, `solidSideFromMotion` (all `const Entity&`) | 3 |
| `isDormant` | `PlayState.cpp` 213, 233, 290, 291, 297, 322, 413, 432 | 8 |

**One file for the motion members (`CollisionManager.cpp`), ~16 call sites.** No content class,
renderer, or enemy behaviour file is affected.

##### `isDormant` is separable — it is not part of this conflict

Dormancy is not a motion property. It answers *"does this entity participate at all yet?"*, which
`tmp`'s stated rule ("nothing in this interface implies life, motion or input") does not exclude.
`isDormant` can live on `Entity` under **either** design. Decide it with C-10, not here.

##### Trade-off, stated honestly

- **`tmp`'s design** buys a real invariant: `FlagPole` and `Pipe` have no `velocity` field, so no
  code can read or write one by accident. It pays with `dynamic_cast` in the collision loop —
  including pass 2, where casts at lines 196 and 211 run once per entity **pair**, every frame.
- **`feat`'s design** buys a uniform, cast-free loop and less code. It pays semantically: a `Pipe`
  carries a permanently-zero `velocity` and `isGrounded` that mean nothing.

> **Do not decide this on performance.** At ~50 entities `tmp`'s pass-2 casts are ~1,300 per frame,
> roughly 0.3% of a 60fps budget. The cost is real but negligible. Decide on which invariant you
> want, not on speed.

##### Recommended resolution — a synthesis, not a pick

`tmp`'s expensive pass-2 casts exist for exactly one reason:

```cpp
// tmp CollisionManager.cpp:196, 211 — the cast is used ONLY to call isDying()
auto* aCharacter = dynamic_cast<Character*>(a);
if (aCharacter && aCharacter->isDying()) continue;
```

`feat` already hoists `isAlive()` and `isDying()` as virtuals on `Entity` with safe defaults
(see **C-13**). Adopting those virtuals **removes both O(n²) casts entirely.** Therefore:

| Take | From | Effect |
|---|---|---|
| `velocity` / `isGrounded` on `Character` | `tmp` | keeps the type-safety invariant |
| virtual `isDying()` / `isAlive()` on `Entity` | `feat` | eliminates the O(n²) casts in pass 2 |
| `isDormant` on `Entity` | `feat` | not a motion property; belongs there either way |

One O(n) `dynamic_cast` survives in pass 1 — or partition the list into `vector<Character*>` once
per frame and it drops to zero.

**Cost:** rewrite ~16 call sites and 3 helper signatures in `CollisionManager.cpp`. No content
files change on either side. This preserves both branches' design intent rather than discarding
one.

#### C-3. Jump model — game feel

| `tmp` | `feat` |
|---|---|
| `getMaxJumpSpeed()` + `getJumpAccel()` | `getJumpForce()` |
| Impulse, then hold-time acceleration, capped | Single impulse |
| `MaxJumpHoldTime = 0.16f`, `JumpInitialSpeed = -220` | `jumpHeld` edge-detect only |
| `coyoteTime`, `jumpBufferTime` (0.1s each) | — |

`tmp`'s constants are tuned against level geometry: comments document that a full hold rises
≈137px — just over a 4-tile wall (128px), clearly under a 5-tile one (160px) — and that the weak
graze at apex is filtered by a bump-speed gate in `CollisionManager`. Changing this changes what
maps are solvable.

`tmp` also adds coyote time and jump buffering, which `feat` does not have.

**This is a gameplay decision, not a technical one — it should be made by both authors together.**
Affects `Mario.h`, `Luigi.h`, `Character.h`, `Player.h/cpp`.

#### C-4. Level timer ownership

| `tmp` | `feat` |
|---|---|
| Separate `Model/Core/LevelTimer` class | Built into `GameManager` |
| — | `startLevelTimer()`, `tickTimer()`, `getTimeRemaining()`, `isTimeUp()`, `awardTimeBonus()` |
| `CoinsPerLife = 100` | `CoinsPerExtraLife = 50` |
| | `LevelTimeUnits = 400`, `TimeUnitsPerSecond = 2.5f`, `PointsPerTimeUnit = 50` |

`tmp` has the cleaner separation; `feat` has the more complete implementation (idempotent
`awardTimeBonus`, smooth fractional countdown).

**Recommended:** keep `tmp`'s standalone `LevelTimer` class, port `feat`'s logic into it.
**Separately decide:** coins per extra life — 100 or 50.

#### C-5. `PlayState` structure

| `tmp` | `feat` |
|---|---|
| Thin (~30-line header) | Monolithic (~90-line header) |
| Owns `LevelScene` + `LevelClearSequence` | Owns map, entities, camera, collision, renderers directly |
| Extracted `LevelScene`, `LevelCompletion`, `PortalSystem`, `LevelClearSequence` | all inline |
| — | **also implements `model::World`** (`spawn`, `getPlayer`) |

These are no longer the same file, hence 409 of 495 lines conflicting.

`tmp`'s decomposition is better structured. But `feat`'s `PlayState : public model::World` provides
runtime spawning (see C-9), which must be preserved regardless — if `tmp`'s structure is adopted,
`LevelScene` becomes the natural `World` implementor instead of `PlayState`.

**Also differs:** `GameManager` in `tmp` carries `currentMapPath` / `nextMapPath` / `levelName` /
`levelClearBonus` for map chaining; `feat`'s does not. `tmp`'s map chaining depends on these.

---

### TIER 3 — False conflicts. Union both sides; no decision needed.

#### C-6. `TileMap` — extended in different directions

| `tmp` added | `feat` added |
|---|---|
| Metadata header (`;` lines): level name, `WorldType`, `next=` chaining | `SpawnPoint` struct + enemy spawn parsing |
| `loadFromLines()` for multi-area maps | `getSpawnPoints()`, `getCoinBlockSpawns()` |
| `padRight()` + castle painting (`CastleSymbols`, 21 tiles) | `breakTile()` for breakable bricks |
| `setTile()`, `isCastleSymbol()` | `tileOrigin()` helper |
| `getLevelName/getWorldType/getNextMapPath/hasNextMap` | `BrickSymbol = 'B'`, `CoinBlockSymbol = 'C'` |

Almost entirely non-overlapping. Union them.

> ⚠️ **Verify the symbol table.** `tmp`'s castle uses
> `A D F H I J L N Q R S U V W X Y Z a b c d` and its comment already reserves `'B'`, `'C'`, `'G'`,
> `'O'`, `'T'`, `'M'`, `'E'`, `'K'`, `'#'`, `'P'`, `'p'`. `feat`'s `B`/`C` therefore appear
> compatible — but confirm against the actual map files rather than assuming.

#### C-7. Gravity scaling — orthogonal axes

- `tmp`: **per-world** — `World::getGravityScale()`, `getMaxFallScale()`, `getHorizontalDrag()`
- `feat`: **per-entity** — `Character::gravityScale` (Lakitu hovers at 0.0, buoyancy, lazy arcs)

Not competing. Keep both:
`finalGravity = DefaultGravity × worldTheme.gravityScale × entity.gravityScale`

#### C-8. View layer — different sub-problems

| `tmp` | `feat` |
|---|---|
| `AssetManager`, `SpritePainter`, `RenderContext`, `TextUtils`, `HudData` | `AtlasFrameRenderer`, `EnemyAtlas`, `ItemAtlas`, `ItemFrameRenderer` |
| infrastructure / plumbing | content / atlas handling |

Both build on the shared `EntityRendererRegistry` and `SpriteEntityRenderer`. Complementary —
union them. `HudRenderer` conflicts (3 hunks) because `tmp` routes through `HudData` while
`feat` reads `GameManager` directly; pick `tmp`'s `HudData` indirection and feed it from the
merged timer.

---

### TIER 4 — Purely additive. Port as-is, no decision.

| # | Feature | From | Notes |
|---|---|---|---|
| C-9 | **Runtime entity spawning** | `feat` | `World::spawn()` + `pendingEntities` deferred splice. **`tmp` has no equivalent** — its entity list is built once at level-build time. Required for hammers, fireballs, transformations. Verified absent from `tmp`. |
| C-10 | **Dormancy / camera activation** | `feat` | `isDormant`, `activationFrontier` (monotonic, so backtracking never re-arms), `ActivationMargin = 64`. No `tmp` equivalent. |
| C-11 | **Power-up body sizing** | `feat` | `syncPowerSize()`, `SmallHeight 32` / `BigHeight 64` / `CrouchHeight 44`, feet-anchored so growth never clips through the floor. Plus `Crouch` in `AnimState`. `tmp` has power *states* but not the resize. |
| C-12 | **Const power queries for view** | `feat` | `isBig()`, `isFire()`, `isStar()`, `isLuigi()`, `getBlinkRemaining()` — let renderers take `const Player&` without `dynamic_cast`. |
| C-13 | **Entity behaviour virtuals** | `feat` | `takeDamage`, `onStomped`, `onHit`, `getDamageValue`, `isAlive`, `isDying`, `handleInput` hoisted to `Entity`. |
| C-14 | **`PlayerState::canShoot()/shoot()`** | `feat` | Fire-flower shooting hook. |
| C-15 | **Level structure suite** | `tmp` | `Level`, `Pipe`, `PortalSystem`, `FlagPole`, clear sequence, map chaining. |
| C-16 | **`onTriggerEnter` hook** | `tmp` | Trigger-hitbox pass in `CollisionManager`. |

> **No conflict:** `usesTileCollision`, `isStompable`, `canBreakBricks`, `drawsBehindTerrain` are
> **byte-identical on both branches** (same code, same comments — inherited from the `37c966e`
> merge). They will not conflict.

---

## 4. Recommended order of work

1. **C-1 — rename the duplicate `World`.** Independent of everything else, and prevents a build
   failure that has no conflict marker to lead you to it. Do it before merging anything.
2. **Decide C-2** (velocity ownership). It shapes the `CollisionManager` signatures the rest of
   the merge is written against — but see the synthesis option; it is ~16 call sites in one file,
   not a branch-wide rewrite.
3. **Decide C-3** (jump model) — both authors together; it is game feel, not engineering.
4. **Decide C-4 / C-5** (timer home, `PlayState` structure) — follow from C-2.
5. Union the Tier-3 items (C-6, C-7, C-8).
6. Port the Tier-4 items (C-9 … C-16) — mechanical.
7. Rebuild from a clean configure, then play-test the jump arcs against existing maps.

---

## 5. Build note (unrelated to the merge, but will bite during it)

`CMakeLists.txt` uses `file(GLOB_RECURSE SOURCES src/*.cpp)`. A glob is evaluated **at configure
time only**, so switching between these branches — which move and add many `.cpp` files — leaves a
stale file list and produces errors like:

```
cc1plus.exe: fatal error: src\Model\Block.cpp: No such file or directory
```

Adding `CONFIGURE_DEPENDS` to the glob makes CMake re-glob on every build and fixes this:

```cmake
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
```

Also note `CMakeUserPresets.json` is **tracked in git** despite being listed in `.gitignore`
(gitignore does not apply to already-tracked files). It hardcodes one machine's SFML path
(`C:/msys64/ucrt64`), which breaks configure for anyone whose install differs. Untrack it:

```sh
git rm --cached CMakeUserPresets.json
```

---

## 6. Bottom line

Only **four** items are genuine decisions: **C-2, C-3, C-4, C-5**. Everything else is a rename,
a union, or a straight port.

Both branches did substantial, legitimate architectural work in different directions — `feat`
toward entity content and runtime systems, `tmp` toward level structure and view infrastructure.
Neither is redundant, and the finished game needs both halves.

**C-2 is smaller than it first appears.** It is confined to ~16 call sites in `CollisionManager.cpp`
— no content class on either side is affected — and it has a synthesis resolution that keeps both
branches' design intent rather than discarding one. Do not treat it as a winner-take-all vote.

The one item that genuinely needs both authors in the room is **C-3**, the jump model: it is game
feel, and it changes which maps are solvable.
