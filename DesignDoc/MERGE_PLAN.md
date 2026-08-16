# Merge Plan — `feat/adding-new-entities-state-movements` ↔ `tmp`

**Authoritative decision-and-implementation document.** Supersedes the three earlier drafts.

| | |
|---|---|
| **OURS** | `feat/adding-new-entities-state-movements` @ `7e9ae6e` — *"Added coin block appear effect"* |
| **THEIRS** | `tmp` @ `39bcf13` — *"k.2.6"* |
| Common ancestor | `c74283c` — *"feat: add new entities and player state movements"* (2026-08-05) |
| Currently checked out | `feat` |

> [!WARNING]
> **The two received reports use the opposite convention.**
> `Conflict_report_Claude.md` and `Merge_suggestion_Claude.md` were written from the **`tmp`
> author's** seat: they define `"YOUR branch" = 39bcf13` = **tmp**. Every *"YOURS wins"* in those
> documents therefore means **theirs**, and every *"THEIRS"* means **ours**. This document uses
> OURS = `feat` throughout. When cross-referencing, invert their labels.

> [!NOTE]
> **The recommendations below are unchanged from the neutral technical analysis.** Relabelling the
> seats does not change which design wins an argument. On structure the evidence favours `tmp`; on
> content and runtime systems it favours `feat`. Both halves are needed.

---

## 1. Corrections to the received reports

Four claims in `Merge_suggestion_Claude.md` / `Conflict_report_Claude.md` were checked against the
source and do not hold. They are listed here because three of them change a decision.

### 1.1 ❌ "Item, Projectile, Coin, Fireball are not Characters" — false

Their Section 1 justifies keeping `velocity` on `Entity` with:

> *"their new `Item`, `Projectile`, `Coin`, `Fireball` classes all need velocity and ground state
> but are **not** Characters. Moving those to `Character` breaks the entire item/projectile layer."*

Verified — **every one of them is a `Character`:**

```
Character  : Entity          Enemy      : Character      Item       : Character
Player     : Character       NPC        : Character      Projectile : Character
Block      : Entity   ← the ONLY direct Entity subclass in feat, and it is static

Coin, Mushroom, FireFlower, Starman        : Item        → Character
Fireball, MarioFireball, Hammer, SpinyEgg  : Projectile  → Character
Goomba, Koopa, Bowser, Lakitu, HammerBro, PiranhaPlant, Spiny : Enemy → Character
```

Our own `Projectile.h` states it: *"Derives from Character to reuse gravity and velocity
integration."* `Item.h` says the same.

**Nothing in our content layer breaks under either design.** Their conclusion may still be
arguable, but not for the reason given. See **D-1**.

### 1.2 ❌ Their Section 1 contradicts itself on `isAlive` / `isDying`

- Decision text: *"`handleInput / takeDamage / onStomped / onHit / getDamageValue / **isAlive /
  isDying*** → REMOVE from `Entity`"*
- "Resolved shape" block, four lines later: *"Entity virtuals: … isSolid, **isAlive, isDying**, …"*

Opposite instructions in one section. This is not cosmetic: `isDying()` on `Entity` is exactly what
removes the O(n²) `dynamic_cast` from `tmp`'s collision pass 2. See **D-1**.

### 1.3 ❌ The `model::World` name collision was missed, and misattributed

Their conflict report lists `World.h` under **"Files ONLY In Their Branch (no conflict — safe)"**,
and Section 1 calls it *"your `World` interface class from `include/Model/Core/World.h`"* —
attributing our file to `tmp`.

There are **two unrelated classes both named `model::World`:**

| `tmp` — `include/Model/World/World.h` | `feat` — `include/Model/Core/World.h` |
|---|---|
| Concrete **theme + physics descriptor** | Pure virtual **service interface** |
| `getBackgroundColor()`, `getTilesetPath()`, `getGravityScale()`, `getMaxFallScale()`, `getHorizontalDrag()` | `spawn(unique_ptr<Entity>)`, `getPlayer()` |
| Built by the `WorldSet` registry | Implemented by a controller |

Follow their own recommendations and the collision becomes live: their Section 1 keeps
`Entity::setWorld(World*)` (ours, the spawn interface) **and** their Section 2 keeps
`Character::setWorld(const World&)` (theirs, the theme). Both survive, under one name, in one
namespace.

**Because the file paths differ, git reports zero conflicts and merges both in silently.** The
build then fails with nothing pointing at the cause. This is **D-4**, and it must be fixed first.

### 1.4 ⚠️ Two smaller inaccuracies

- **`CMakeLists.txt`** is listed as *"only in their branch — safe."* It is on **both** branches and
  differs (**+11 / −3**). See §6.
- **Trace logging** — they say remove it from *"CoinBlock.cpp and Player.cpp."* It is in **7 files,
  28 call sites**, all on `tmp`: `AppEngine`(4), `LevelScene`(11), `Player`(5),
  `LevelClearSequence`(2), `BrickBlock`(2), `CoinBlock`(2), `Goomba`(2).

---

## 2. Scope

| | |
|---|---|
| `tmp` since ancestor | 14 commits · 101 files · **+4,682 / −1,444** |
| `feat` since ancestor | 7 commits · 82 files · **+3,087 / −323** |
| Files edited on both sides | **31** |
| Files that conflict | **26** |
| Conflict hunks | **65** (~1,800 conflicted lines) |

`tmp` already merged an earlier `feat` at `37c966e`, then spent 13 commits (`k.2.0`→`k.2.6`)
refactoring. `feat` kept building on the pre-refactor base. That is the whole source of divergence.

**Worst files:** `PlayState.cpp` (8 hunks, 409/495 lines), `CollisionManager.cpp` (8 hunks,
362/653), `PlayState.h` (4 hunks), `Character.cpp` (4 hunks).

### What each side uniquely contributes

**Ours (`feat`) — 48 new files: content and runtime systems**
- 5 enemies (`Bowser`, `Lakitu`, `HammerBro`, `PiranhaPlant`, `Spiny`) + `EnemyFactory`
- 5 projectiles (`Fireball`, `MarioFireball`, `Hammer`, `SpinyEgg`, `Projectile`)
- 5 items (`Mushroom`, `FireFlower`, `Starman`, `Coin`, `Item`)
- Runtime spawning (`World::spawn`), dormancy/camera activation, power-up body sizing, crouch
- View: `AtlasFrameRenderer`, `EnemyAtlas`, `ItemAtlas`, `ItemFrameRenderer`, `FireballRenderer`

**Theirs (`tmp`) — 48 new files: level structure and view infrastructure**
- `Level` (multi-area), `Pipe` + `PortalSystem`, `FlagPole` + castle painting
- `LevelScene`, `LevelClearSequence`, `LevelCompletion`, `LevelCompleteState`, `LevelTimer`
- `WorldSet`, `WorldType`, `BrickBlock`
- View: `AssetManager`, `SpritePainter`, `RenderContext`, `TextUtils`, `HudData`

Neither is a subset of the other. The finished game needs both.

---

## 3. Settled — no decision required

All three analyses agree. Implement as stated.

| Item | Take from | Note |
|---|---|---|
| Movement model — acceleration/inertia | **theirs** | Eliminates teleporting-stop feel; jump-hold math needs per-frame `dt` |
| Coyote time + jump buffering | **theirs** | 0.1s each; we have no equivalent |
| `handleInput(float dt)` signature | **theirs** | Required by the acceleration math |
| `score` / `coins` owned by `GameManager` only | **theirs** | *Our own* `GameManager` comment argues this: the player is rebuilt on every death |
| Separate `LevelTimer` class | **theirs** | Do not embed the timer in `GameManager` |
| `LevelScene` / `LevelClearSequence` extraction | **theirs** | `PlayState` stays thin |
| Map paths, `levelName`, `levelClearBonus` | **theirs** | Required by `LevelCompleteState` + map chaining |
| `onBlockHit(BlockHitEvent&)` over `onCollision` | **theirs** | Event carries bumper + side; no re-derivation |
| Gravity constant rename → `DefaultGravity` / `DefaultMaxFallSpeed` | **theirs** | Marks them as baseline, not absolute |
| `onTriggerEnter` hook | **theirs** | Used by `FlagPole` |
| `usesTileCollision` / `isStompable` / `canBreakBricks` / `drawsBehindTerrain` | **either** | **Byte-identical on both branches** (from the `37c966e` merge) — will not conflict |
| `World::spawn()` runtime spawning | **ours** | `tmp` has **no** runtime spawn channel — verified. Required for hammers, fireballs, item drops |
| Dormancy / camera activation (`isDormant`, `activationFrontier`) | **ours** | Monotonic frontier; backtracking never re-arms |
| Power-up body sizing (`syncPowerSize`, `SmallHeight/BigHeight/CrouchHeight`) | **ours** | Feet-anchored; growth never clips through floor |
| `AnimState::Crouch` + crouch system | **ours** | Integrates with inertia model (`crouch` locks `horizontalInput = 0`) |
| `isBig` / `isFire` / `isStar` / `isLuigi` / `getBlinkRemaining` | **ours** | Const queries; `isLuigi()` avoids `dynamic_cast` in the renderer |
| `gravityScale` per-entity | **ours** | Lakitu hover, PiranhaPlant pin, fireball arc. Coexists with per-world scaling |
| Enemy `createProjectile()` / `getScoreValue()` / `awardScore()` / `findPlayer()` | **ours** | Factory Method; `findPlayer` goes through `world->getPlayer()`, no type-checking |
| `PlayerState::canShoot()` / `shoot()` | **ours** | Fire-flower hook |
| `deathElapsed` | **ours** | **Verified absent from `tmp`** — must be restored |
| `addCoin(int count = 1)` | **ours** | Default arg; all existing `addCoin()` call sites compile unchanged |
| `setMap()` | **theirs** | We removed it; tile collision still needs the map pointer |
| `TileMap` — union both sides | **both** | See §5.3 |
| Per-world physics (`getGravity/getMaxFallSpeed/isUnderwater/horizontalDrag`) | **theirs** | Reads from the theme descriptor |
| Overlap epsilon check in `CoinBlock` | **ours** | Correctness fix, independent of other decisions |
| Random `?`-block rewards (the feature) | **ours** | The table itself is **D-8** |
| Remove all `trace()` logging | **theirs' code** | 7 files, 28 sites — see §1.4 |

---

## 4. Decisions — LOCKED

> Settled by the `feat` author. The rationale for each is kept below for reference, but the
> outcomes are no longer open. Implementation tasks: see **`MERGE_TASKS.md`**.

| # | Decision | Outcome |
|---|---|---|
| **D-1** | `velocity` / `isGrounded` ownership | **Option C** — on `Character` (theirs) + virtual `isDying()`/`isAlive()` on `Entity` (ours) |
| **D-2** | Jump model | **Theirs** — `getMaxJumpSpeed()` + `getJumpAccel()`, coyote time, jump buffering |
| **D-3** | `model::World` implementor | **Synthesis** — `LevelScene`, not `PlayState` |
| **D-4** | `World` name collision | **Option A** — rename theirs to `WorldTheme`; ours keeps `World` |
| **D-5** | `Mario::RunSpeed` | **360** (theirs) — tuned against Overworld drag |
| **D-6** | Coins per extra life | **100** (theirs) — canonical SMB1 |
| **D-7** | `FireCooldownDuration` | **0.5s** (theirs) |
| **D-8** | `?`-block reward table | **75% coin / 15% mushroom / 5% flower / 5% star** — feature kept, item rate cut |
| **D-9** | Damage window | **Unified** — one constant drives invulnerability *and* blink, per our design intent. Start at **1.0s** |
| — | Level timer | **Theirs** — `LevelTimer` class, **400 real seconds**. Port our `awardTimeBonus()` into it |

**Physics generally follows `tmp`:** movement model, acceleration/inertia, animation thresholds,
gravity constants, jump tuning.

---

## 4b. Rationale (reference)

### D-1. `velocity` / `isGrounded` ownership ⭐ *highest impact*

Both branches store the level as one heterogeneous container:

```cpp
std::vector<Entity*> entities;   // Blocks, Pipes, FlagPole, Player, Enemies, Items, Projectiles
```

The entire conflict is **how `CollisionManager` reaches `velocity` from an `Entity*`.**

```cpp
// OURS — fields on Entity, no cast
entity->isGrounded = false;                 // CollisionManager.cpp:115
Vector2 vel = entity->getVelocity();        // :128

// THEIRS — fields on Character, must downcast
auto* character = dynamic_cast<Character*>(entity);          // :88
if (!character || character->isDying()) continue;
character->isGrounded = false;
processTileCollisions(*character, deltaTime);
```

**Measured blast radius** (every site in `feat` reaching these through an `Entity`):

| Member | File · lines | Count |
|---|---|---:|
| `isGrounded` | `CollisionManager.cpp` 115, 156, 365 | 3 |
| `getVelocity` / `setVelocity` | `CollisionManager.cpp` 39, 60, 77, 128, 236, 330, 364, 369, 375, 380 | 10 |
| helper signatures | `isStompFromAbove`, `hasStableTopContact`, `solidSideFromMotion` | 3 |

**One file, ~16 call sites.** No content class, enemy, item, projectile, or renderer is affected.

| | **A — on `Entity`** (ours) | **B — on `Character`** (theirs) | **C — Synthesis** ✅ |
|---|---|---|---|
| `velocity` / `isGrounded` | `Entity` | `Character` | `Character` *(theirs)* |
| `isDying()` / `isAlive()` | virtual on `Entity` | on `Character` | virtual on `Entity` *(ours)* |
| `dynamic_cast` in pass 2 | none | **O(n²)** — once per entity *pair*, per frame | **none** |
| `Pipe`/`FlagPole` carry dead `velocity` | yes | no | **no** |
| Cost | `tmp` reworks its collision layer | ~16 sites, 1 file | ~16 sites, 1 file |

**Why C works:** `tmp`'s pass-2 casts (lines 196, 211) exist **only** to call `isDying()`. We
already hoist `isDying()`/`isAlive()` as virtuals on `Entity` with safe defaults. Adopting those
removes both casts entirely, so `tmp` keeps its type-safety invariant *and* loses the cast overhead.
One O(n) cast survives in pass 1 — or partition into `vector<Character*>` once per frame for zero.

> **Do not decide this on performance.** At ~50 entities the pass-2 casts are ~1,300/frame, about
> 0.3% of a 60fps budget. Real, negligible. Decide on which invariant you want.

**Recommendation: C.** Neither received report proposed it.

**`isDormant` is separable.** Dormancy is not a motion property — it answers *"does this entity
participate yet?"*, which `tmp`'s stated rule ("nothing in this interface implies life, motion or
input") does not exclude. It stays on `Entity` under **any** option above. Not part of D-1.

---

### D-2. Jump model

| Ours | Theirs |
|---|---|
| `getJumpForce()` — single impulse | `getMaxJumpSpeed()` + `getJumpAccel()` |
| `Mario::JumpForce = -600`, `Luigi = -720` | `MaxJumpSpeed = 680`, `JumpAccel = 3100` |
| `jumpHeld` edge-detect | `MaxJumpHoldTime = 0.16f`, `JumpInitialSpeed = -220` |

Their constants are tuned against level geometry: a full hold rises ≈137px — just over a 4-tile wall
(128px), clearly under a 5-tile one (160px) — and the weak graze at apex is filtered by a bump-speed
gate in `CollisionManager`.

**Recommendation: theirs.** A single impulse cannot do variable-hold jumps, and the hold logic
already needs `dt`, which D-3-settled `handleInput(float)` provides.

> This is the one item that changes **which maps are solvable**. It is game feel, not engineering —
> play-test it together rather than deciding on paper.

---

### D-3. Who implements `model::World`

| Ours | Theirs | Synthesis ✅ |
|---|---|---|
| `PlayState : public model::World` | `World` not implemented (no spawn channel) | **`LevelScene` implements `World`** |

`PlayState : public model::World` puts a controller class inside the model layer's abstraction
boundary. `LevelScene` already owns the entities, the camera, and `pendingEntities` — it is the
natural implementor.

**Recommendation: the synthesis** (this one comes from their Section 4 and is correct). Move
camera, `armDormancy()`, and the `pendingEntities` splice into `LevelScene`. Keep the `playAsLuigi`
toggle in `PlayState` — it is controller-level input, passed in at scene construction.

Depends on **D-4** being resolved first.

---

### D-4. The `model::World` name collision ⚠️ *blocking — fix before merging anything*

Two unrelated classes, one name, different paths → **git merges both with no conflict marker.**

| Option | Action | Effect |
|---|---|---|
| **A** ✅ | Rename **theirs** → `WorldTheme` | Ours keeps `World`. Matches their header's own wording: *"immutable descriptors"*. Their `WorldSet`/`WorldType` follow naturally |
| **B** | Rename **ours** → `LevelContext` | Their `World`/`WorldSet`/`WorldType` trio stays symmetric; our ~5 referencing files change |
| **C** | Separate namespaces | More churn, no more clarity |

**Recommendation: A.** Update `WorldSet::forType()`, `Character::setWorld/worldPtr`, and the
renderer's theme lookup. Independent of every other decision — do it on day one.

---

### D-5 … D-9. Gameplay tuning — pure judgement calls

| # | Setting | Ours | Theirs | Note |
|---|---|---|---|---|
| **D-5** | `Mario::RunSpeed` | **400** | **360** | Theirs documents 360 as tuned against Overworld drag (~350px/s effective). Ours is the SMB original |
| **D-6** | Coins per extra life | **50** (`CoinsPerExtraLife`) | **100** (`CoinsPerLife`) | 100 is canonical SMB1. Ours was justified as *"this clone is shorter"* |
| **D-7** | `FireCooldownDuration` | **1.0s** | **0.5s** | Both verified in source |
| **D-8** | `?`-block reward table | 40% coin / 30% mushroom / 15% flower / 15% star | always coin | **60% power-up per block is very high.** Keep our *feature*, retune the table — e.g. 70/20/5/5 |
| **D-9** | Damage window | `DamageBlinkTime = 2.0f` | `DamageCooldownTime = 0.5f` | See below |

**On D-9:** their Section 3 recommends keeping both as separate concepts (0.5s invulnerability,
2.0s blink). **This contradicts our branch's own stated design** — our `Player.h` comment says the
blink and the invulnerability window must end together *"so a blinking Mario can never be hit after
the window the animation implies has run out."* Pick **one** duration and drive both from it.

**Also unresolved — timer semantics.** Their `LevelTimer` counts **400 real seconds**
(`LevelTimer(float startSeconds = 400.0f)`). Ours counts **400 units at 2.5 units/sec = 160 real
seconds**. Same displayed number, 2.5× different play length. Decide which, then port
`awardTimeBonus()` / `PointsPerTimeUnit` (ours) into their `LevelTimer` class (D-settled §3), which
currently has no bonus conversion.

---

## 5. Implementation notes per area

Ordered by dependency. Each unlocks the next.

### 5.0 `World` rename — do first
Apply **D-4 Option A**. No merge conflicts to resolve; pure rename on the `tmp` side.

### 5.1 `Entity.h` — resolved shape

Under **D-1 Option C**:

```
Entity owns:      position, size, hitbox, isActive, isDormant, world*
Entity virtuals:  update, onCollision, onTileCollision, onTriggerEnter,
                  isSolid, isAlive, isDying,
                  usesTileCollision, isStompable, canBreakBricks, drawsBehindTerrain
Entity does NOT:  velocity, isGrounded, handleInput, takeDamage,
                  onStomped, onHit, getDamageValue        ← these live on Character / Enemy
```

`Character` gains `velocity`, `isGrounded` (as `tmp` has them) and keeps our `gravityScale`,
`deathElapsed`, `AnimState::Crouch`.

### 5.2 `CollisionManager.cpp` — the ~16 sites

Rewrite the sites tabulated in D-1. Take `tmp`'s file as the base (it is the newer architecture),
then re-apply our additions on top: dormancy checks, `usesTileCollision()` gate, `isStompable()`
routing, `canBreakBricks()` brick smashing, `breakTile()` calls, coin spawn on brick break.
Replace both pass-2 `dynamic_cast`s with direct `isDying()` virtual calls.

### 5.3 `TileMap` — union, no conflict in substance

| Theirs adds | Ours adds |
|---|---|
| Metadata header (`;` lines): `levelName`, `WorldType`, `next=` chaining, `parseHeader()` | `SpawnPoint` struct + enemy spawn parsing |
| `loadFromLines()` for multi-area maps | `getSpawnPoints()`, `getCoinBlockSpawns()` |
| `padRight()` + castle painting (`CastleSymbols`, 21 tiles) | `breakTile()` |
| `setTile()`, `isCastleSymbol()` | `tileOrigin()`, `BrickSymbol='B'`, `CoinBlockSymbol='C'` |

**Symbol table — verified safe.** Their castle uses
`A D F H I J L N Q R S U V W X Y Z a b c d`; their comment already reserves `'B'`, `'C'`, `'G'`,
`'O'`, `'T'`, `'M'`, `'E'`, `'K'`, `'#'`, `'P'`, `'p'`. Our `B`/`C` do not collide.
Confirm against the actual `.map` files before relying on it.

Declare `SpawnPoint` before `class TileMap`.

### 5.4 `GameManager`
Take their field set (map paths, `levelName`, `levelClearBonus`) + our `addCoin(int count = 1)`.
**Do not** embed the timer. Resolve **D-6** (coins-per-life) and the timer-semantics question.

### 5.5 `Enemy.h/.cpp`
Keep all our additions (`createProjectile`, `getScoreValue`, `awardScore`, `updateAttack`,
`findPlayer`, cooldown fields). Under 5.1, `onHit`/`getDamageValue` are first declared on
`Character`/`Enemy`, so **drop the `override` keyword** on them.

### 5.6 `Player.h/.cpp`
Their movement model + coyote/buffer + `handleInput(float dt)`; our crouch, `syncPowerSize`,
power queries, blink, `horizontalInput`/`sprinting`/`animationClock`. Remove `score`/`coins` fields;
keep the thin `GameManager` wrappers. Resolve **D-2**, **D-5**, **D-7**, **D-9**.

### 5.7 `CoinBlock.h/.cpp`
Their `onBlockHit(BlockHitEvent&)` trigger and animation API
(`isOpened`/`isCoinPopping`/`getCoinPopProgress`, `coinPopElapsed`); our random reward spawning via
`world->spawn()`, moved inside `onBlockHit`. Keep our overlap epsilon check. Resolve **D-8**.

> **On the `dynamic_cast<Player*>`** we use for mushroom direction: their Section 6 wanders through
> three different fixes and lands on putting `getDirection()` on `Entity`. That contradicts the ISP
> argument driving 5.1. Simpler: `BlockHitEvent` already carries the bumper reference, and only a
> `Character` can bump a block — take the direction from `Character`, which already has
> `getDirection()`. No new `Entity` surface needed.

### 5.8 `PlayState` / `LevelScene`
Apply **D-3**. `LevelScene` becomes the `World` implementor and absorbs camera, `armDormancy()`,
and the `pendingEntities` splice. `PlayState::update()` reduces to `scene->update(dt)` plus HUD
and state transitions.

### 5.9 View layer — union
Theirs: `AssetManager`, `SpritePainter`, `RenderContext`, `TextUtils`, `HudData`.
Ours: `AtlasFrameRenderer`, `EnemyAtlas`, `ItemAtlas`, `ItemFrameRenderer`, `FireballRenderer`.
Both build on the shared `EntityRendererRegistry` / `SpriteEntityRenderer`.
`HudRenderer` conflicts because theirs routes through `HudData` while ours reads `GameManager`
directly — **take the `HudData` indirection** and feed it from the merged timer.

### 5.10 Cleanup
Remove all 28 `trace()` calls across the 7 `tmp` files (§1.4), the `#include <fstream>`, and the
anonymous `trace()` helpers. `trace_log.txt` and `log.txt` are already gitignored.

---

## 6. Build note

`feat`'s `CMakeLists.txt` **already carries the `CONFIGURE_DEPENDS` fix** and is the better of the
two — it uses `${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp` and documents the failure mode:

```cmake
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
```

**Take ours wholesale.** Without it, a plain `GLOB_RECURSE` is evaluated only at configure time, so
switching between these branches — which move and add many `.cpp` files — leaves a stale file list
and produces errors like:

```
cc1plus.exe: fatal error: src\Model\Block.cpp: No such file or directory
```

Separately: **`CMakeUserPresets.json` is tracked in git** despite being listed in `.gitignore`
(gitignore does not apply to already-tracked files). It hardcodes one machine's SFML path
(`C:/msys64/ucrt64`), breaking configure for anyone whose install differs:

```sh
git rm --cached CMakeUserPresets.json
```

---

## 7. Order of work

```
1.  D-4  World rename ......................... blocking, independent, do first
2.  D-1  velocity ownership (→ Option C) ...... sets CollisionManager signatures
3.  5.1  Entity.h resolved shape
4.  5.2  CollisionManager.cpp (~16 sites)
5.  5.3  TileMap union ........................ independent; unlocks LevelScene
6.  5.4  GameManager .......................... independent service layer
7.  5.5  Enemy ................................ needs Entity + Character settled
8.  5.6  Player ............................... needs Character + GameManager
9.  5.7  CoinBlock ............................ needs world ptr + Character direction
10. 5.8  PlayState / LevelScene (D-3) ......... integrates everything
11. 5.9  View layer union
12. 5.10 Remove trace logging
13.      Clean reconfigure, rebuild, play-test jump arcs against existing maps
```

---

## 8. Verification checklist

- [ ] Exactly **one** `model::World` symbol exists after the rename
- [ ] `grep -rn "dynamic_cast" src/Model/Core/CollisionManager.cpp` → at most one, in pass 1
- [ ] No `velocity` / `isGrounded` on `Pipe`, `FlagPole`, `Block`
- [ ] `deathElapsed` restored on `Character`
- [ ] Zero `trace(` call sites remain
- [ ] `CMakeUserPresets.json` untracked
- [ ] Clean configure from an empty `build/` succeeds
- [ ] Mario clears a 4-tile wall and fails a 5-tile wall (D-2 regression)
- [ ] A `?` block can drop each of coin / mushroom / flower / star
- [ ] Hammer Bro throws; Fire Mario shoots (proves `World::spawn` survived)
- [ ] Enemies wake on camera approach and do not re-arm when backtracking

---

## 9. Bottom line

Only **four** items are genuine architecture decisions — **D-1, D-2, D-3, D-4** — and two of them
(D-3, D-4) have a synthesis that costs almost nothing. **D-1 is ~16 call sites in one file**, not a
branch-wide rewrite; the received report's claim that it "breaks the entire item/projectile layer"
is false.

The remaining five (D-5 … D-9) are gameplay tuning, not engineering.

On structure the evidence favours `tmp`; on content and runtime systems it favours `feat`. Neither
branch is redundant, and **`tmp` cannot ship a hammer, a fireball, or an item drop without our
`World::spawn`** — just as we cannot ship a flagpole, a pipe, or a level transition without theirs.

**D-2 is the only item that should not be decided by one person on paper.** It changes which maps
are solvable.
