# Merge Tasks — `tmp` × `feat`

Execution plan for the decisions locked in **`MERGE_PLAN.md` §4**.
Three tasks, each ending at a **checkpoint that builds and runs**.

| | |
|---|---|
| **OURS** | `feat/adding-new-entities-state-movements` @ `7e9ae6e` |
| **THEIRS** | `tmp` @ `39bcf13` |
| Integration branch | `integration/merge-tmp-feat` — **branched from `tmp`** (see below) |

> [!IMPORTANT]
> **Why branch from `tmp` and not from `feat`.**
> The locked decisions take `tmp`'s architecture for `Entity`, `Character`, `CollisionManager`,
> `Player` movement, `PlayState`/`LevelScene` and all physics. Those are exactly the 26 conflicting
> files. Our unique contribution is 48 **new** files that conflict with nothing.
> Starting from `tmp` and adding our content turns ~65 conflict hunks into ~48 clean file copies
> plus a handful of targeted edits. Starting from `feat` would mean hand-resolving all 65 hunks to
> arrive at the same result.
> **No work is lost either way** — `feat` stays intact on origin, and every one of our files lands
> in the integration branch.

### Testing reality

There is **no automated test suite** in this project — no test target in `CMakeLists.txt`, no test
directory, no framework on either branch. Every checkpoint below is therefore **build verification
plus scripted manual play-test**. Debug keys available in `PlayState`:

| Key | Effect |
|---|---|
| `Esc` | back to menu |
| `G` | kill the player through the normal death flow |
| `C` | switch Mario/Luigi, restart level *(ours)* |
| `H` | toggle hitbox overlay |
| `N` | bank time bonus + restart *(ours)* |

> [!NOTE]
> **Pre-existing bug to fix in Task 3.** In our `PlayState::handleEvent`, `case Key::H` has **no
> `break`** and falls through into `case Key::N` — pressing `H` toggles hitboxes *and* awards the
> time bonus and resets the level. See `src/Controller/PlayState.cpp:259-266`.

### Build command (all checkpoints)

```sh
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/SFML/SFML-3.0.2" -DSFML_STATIC_LIBRARIES=ON
cmake --build build -j 8
build/bin/SuperMario.exe
```

A **clean** reconfigure (`rm -rf build` first) is required at the end of each task, because file
sets change between tasks.

---

# TASK 1 — Foundation: names and the class shape

**Goal:** land `D-4` and `D-1` on a branch that still builds and plays exactly like `tmp` today.
No new content yet. This task is deliberately behaviour-neutral, so any regression is unambiguous.

**Depends on:** nothing.

### 1.1 Create the integration branch
```sh
git checkout -b integration/merge-tmp-feat tmp
```

### 1.2 D-4 — rename their `World` → `WorldTheme`
- `include/Model/World/World.h` → `WorldTheme.h`; `src/Model/World/World.cpp` → `WorldTheme.cpp`
- Class `World` → `WorldTheme`; include guard `MODEL_WORLD_WORLD_H` → `MODEL_WORLD_WORLDTHEME_H`
- Update `WorldSet::forType()` return type, `Character::setWorld/worldPtr/getGravity/...`, and the
  renderer's theme lookup
- Leave `WorldSet` / `WorldType` names unchanged

**After this the name `model::World` is free** for our spawn interface, which arrives in Task 2.

### 1.3 D-1 Option C — resolved `Entity` / `Character` shape

`Entity` gains our virtuals (safe defaults), keeps motion **out**:
```
Entity owns:      position, size, hitbox, isActive
Entity virtuals:  update, onCollision, onTileCollision, onTriggerEnter, isSolid,
                  isAlive, isDying          ← NEW, from ours; defaults: true / false
Entity does NOT:  velocity, isGrounded, handleInput, takeDamage,
                  onStomped, onHit, getDamageValue
```
`Character` keeps `velocity` / `isGrounded` exactly as `tmp` has them. `Character::isDying()` and
`isAlive()` become `override`.

### 1.4 Remove the pass-2 `dynamic_cast`s
In `src/Model/Core/CollisionManager.cpp`, lines **196** and **211**, replace:
```cpp
auto* aCharacter = dynamic_cast<Character*>(a);
if (aCharacter && aCharacter->isDying()) continue;
```
with the direct virtual call:
```cpp
if (a->isDying()) continue;
```
The pass-1 cast at line **88** stays — it still needs a `Character&` for `processTileCollisions`.

### ✅ Checkpoint 1

**Build:** clean reconfigure + build, **zero warnings introduced**.

**Static checks:**
- [ ] `grep -rn "model::World\b" include/ src/` → **no** hits for the theme class; only `WorldTheme`
- [ ] `grep -c "dynamic_cast" src/Model/Core/CollisionManager.cpp` → **exactly 2**
      (pass-1 `Character*` at ~88, and the existing `Block*` at ~352)
- [ ] `grep -n "velocity\|isGrounded" include/Model/Entity.h` → **no matches**

**Play-test — must be indistinguishable from `tmp` before the change:**
- [ ] Level loads; Mario walks, runs, jumps
- [ ] **Jump arc regression:** clears a 4-tile wall, fails a 5-tile wall
- [ ] Stomping a Goomba kills it and bounces the player
- [ ] `G` → death animation plays, life is lost, level restarts
- [ ] `H` → hitbox overlay toggles
- [ ] Flagpole triggers the clear sequence; level-complete overlay appears
- [ ] Pipe entry (hold Down on a pipe) still teleports

> **Stop here and report.** Task 2 should not begin until Checkpoint 1 is green — every later
> failure would otherwise be ambiguous between "rename broke it" and "new content broke it".

---

# TASK 2 — Content layer: our 48 files + the spawn channel

**Goal:** every entity type we wrote lives in the game and can be spawned at runtime.

**Depends on:** Task 1 (needs the free `World` name and the settled `Entity` shape).

### 2.1 Bring in our `World` spawn interface
Copy `include/Model/Core/World.h` from `feat` unchanged. Add `setWorld(World*)` + the protected
`world` pointer to `Entity` (this is a *service channel*, not motion — it belongs on `Entity`).

### 2.2 D-3 — `LevelScene` implements `World`
```cpp
class LevelScene : public model::World {
    void spawn(std::unique_ptr<model::Entity> entity) override;   // → pendingEntities
    const model::Entity* getPlayer() const override;
};
```
- Add `pendingEntities` + the post-update splice (growing `entities` mid-iteration invalidates it)
- `LevelScene` calls `setWorld(this)` on every entity it takes ownership of
- `PlayState` does **not** inherit `model::World`

### 2.3 Port the content files (clean copies — nothing to resolve)
```
Model/Item/        Item, Mushroom, FireFlower, Starman, Coin
Model/Projectile/  Projectile, Fireball, MarioFireball, Hammer, SpinyEgg
Model/Enemy/       Bowser, Lakitu, HammerBro, PiranhaPlant, Spiny, EnemyFactory
View/              AtlasFrameRenderer, EnemyAtlas, ItemAtlas, ItemFrameRenderer, FireballRenderer
assets/            enemies-8.png, Item sprite sheet, characters.paint
```
Rebase note: these already derive from `Character`, so **no hierarchy changes are needed**. Only
`Entity::velocity` → `Character::velocity` access paths, if any remain.

### 2.4 Port the supporting systems
- **Dormancy** — `isDormant` on `Entity`; `activationFrontier` + `armDormancy()` into `LevelScene`
  (not `PlayState`, per D-3); `ActivationMargin = 64`
- **`gravityScale`** on `Character`; final gravity = `DefaultGravity × theme.gravityScale × entity.gravityScale`
- **`deathElapsed`** restored on `Character`
- **Enemy additions** — `createProjectile()`, `getScoreValue()`, `awardScore()`, `updateAttack()`,
  `findPlayer()`, cooldown fields. **Drop `override`** on `onHit`/`getDamageValue` (now first
  declared on `Character`/`Enemy`)
- **`TileMap` union** — add our `SpawnPoint`, `getSpawnPoints()`, `getCoinBlockSpawns()`,
  `breakTile()`, `tileOrigin()`, `BrickSymbol`, `CoinBlockSymbol` alongside their metadata header,
  `loadFromLines()`, `padRight()`, `setTile()`, castle symbols. Declare `SpawnPoint` before `TileMap`
- **`GameManager`** — their field set + our `addCoin(int count = 1)`. **D-6: `CoinsPerLife = 100`.**
  Timer stays in their `LevelTimer` class; port our `awardTimeBonus()` / `PointsPerTimeUnit` into it.
  **400 real seconds** (their semantics)

### ✅ Checkpoint 2

**Build:** clean reconfigure + build.

**Static checks:**
- [ ] `grep -rn "class World" include/` → **exactly one** (`Model/Core/World.h`, the interface)
- [ ] `grep -rn ": public model::World" include/` → **only** `LevelScene`
- [ ] `EnemyFactory` is reachable from the map loader

**Play-test:**
- [ ] Map spawn markers produce the right enemies (Goomba, Koopa, and at least one new type)
- [ ] **Runtime spawn works** — a Hammer Bro throws a hammer *(this is the capability `tmp` lacked)*
- [ ] A Spiny Egg is lobbed and lands
- [ ] Lakitu hovers rather than falling (proves `gravityScale`)
- [ ] Piranha Plant emerges from its pipe and draws *behind* the pipe
- [ ] **Dormancy:** enemies are still until the camera nears, then activate; walking back left and
      right again does **not** re-arm an already-woken enemy
- [ ] Timer counts down from 400 and the level ends at zero
- [ ] Everything from Checkpoint 1 still passes (jump arc, flagpole, pipes)

---

# TASK 3 — Player, blocks, view, tuning, cleanup

**Goal:** the merged game is feature-complete and tuned.

**Depends on:** Task 2.

### 3.1 `Player` — their movement, our abilities
Keep from **theirs**: acceleration/inertia, `GroundAccel`/`AirAccel`/`Friction`, coyote time, jump
buffering, `handleInput(float dt)`, animation hysteresis thresholds, `MaxJumpSpeed`/`JumpAccel`.

Add from **ours**: `crouching` + `syncPowerSize()` + `SmallHeight`/`BigHeight`/`CrouchHeight`,
`isBig()`/`isFire()`/`isStar()`/`isLuigi()`, `canBreakBricks()`, `getBlinkRemaining()`,
`horizontalInput`/`sprinting`/`animationClock`, `PlayerState::canShoot()`/`shoot()`.

Remove `score`/`coins` fields; keep the thin `GameManager` wrappers.
Crouch locks `horizontalInput = 0` so it composes with the inertia model.

### 3.2 `CoinBlock` — their trigger, our rewards
- Trigger: **their** `onBlockHit(BlockHitEvent&)`; animation API `isOpened` / `isCoinPopping` /
  `getCoinPopProgress` / `coinPopElapsed`
- Rewards: **our** random spawn, moved inside `onBlockHit`, via `world->spawn()`
- Keep our overlap epsilon check
- **Replace the `dynamic_cast<Player*>`** used for mushroom direction: `BlockHitEvent` already
  carries the bumper, and only a `Character` can bump a block — read `Character::getDirection()`.
  Do **not** add `getDirection()` to `Entity`

**D-8 — new reward table:**
```cpp
static constexpr float CoinChance     = 0.75f;   // was 0.40
static constexpr float MushroomChance = 0.15f;   // was 0.30
static constexpr float FlowerChance   = 0.05f;   // was 0.15
static constexpr float StarmanChance  = 0.05f;   // was 0.15
```

### 3.3 View layer union
Theirs: `AssetManager`, `SpritePainter`, `RenderContext`, `TextUtils`, `HudData`,
`BrickBlockRenderer`, `FlagPoleRenderer`, `PipeRenderer`.
Ours: the atlas renderers from 2.3, plus Luigi row, crouch frame, blink fade in `PlayerRenderer`.
**`HudRenderer` takes their `HudData` snapshot**, fed from the merged `LevelTimer`.

### 3.4 Apply remaining tuning
| Setting | Value |
|---|---|
| `Mario::RunSpeed` | **360** (D-5) |
| `FireCooldownDuration` | **0.5s** (D-7) |
| Damage window | **one constant, 1.0s**, driving invulnerability *and* blink (D-9) |

> **D-9 note.** The two branches had `0.5s` (invulnerability) and `2.0s` (blink). Unifying at
> `1.0s` keeps the blink legible without making the player near-invincible for two seconds. This is
> an explicit playtest knob — adjust after Checkpoint 3.

### 3.5 Cleanup
- Remove all **28 `trace()` calls across 7 files** (`AppEngine` 4, `LevelScene` 11, `Player` 5,
  `LevelClearSequence` 2, `BrickBlock` 2, `CoinBlock` 2, `Goomba` 2), their `#include <fstream>`,
  and the anonymous `trace()` helpers
- **Fix the `case Key::H` fall-through** (missing `break`) noted above
- `git rm --cached CMakeUserPresets.json`
- Confirm `CMakeLists.txt` keeps our `CONFIGURE_DEPENDS` glob

### ✅ Checkpoint 3 — final acceptance

**Build:** clean reconfigure, **zero warnings**, from a fresh clone if possible.

**Static checks:**
- [ ] `grep -rn "trace(" src/` → **zero**
- [ ] `grep -rn "dynamic_cast<Player\*>" src/` → **zero**
- [ ] `git ls-files CMakeUserPresets.json` → **empty**
- [ ] Exactly one `model::World`; exactly one `WorldTheme`

**Play-test — full matrix:**
- [ ] **Jump arc:** clears 4-tile wall, fails 5-tile wall *(D-2 regression — the one that matters)*
- [ ] Coyote time: jump fires shortly after walking off a ledge
- [ ] Jump buffer: press just before landing → jump fires on contact
- [ ] Crouch as big Mario; feet stay anchored, no clipping through floor
- [ ] Mushroom → Super (grows without falling through the floor); Flower → Fire; Star → invincible
- [ ] Fire Mario shoots; cooldown ≈ 0.5s
- [ ] Big Mario smashes a brick; small Mario bounces off
- [ ] `?` block drops each of the four rewards over ~20 hits, **mostly coins**
- [ ] Damage → blink and invulnerability begin and end **together**
- [ ] `C` switches Mario/Luigi; Luigi uses his own sprite row and jump values
- [ ] `H` toggles hitboxes **and does nothing else** (fall-through fixed)
- [ ] Flagpole → clear sequence → level-complete overlay → next map loads
- [ ] Pipe/portal transitions work
- [ ] Timer expiry kills the player; `N` banks the bonus
- [ ] Death → life lost → restart; at zero lives → Game Over
- [ ] HUD shows score, coins, lives, timer, all updating

---

## Summary

| Task | Scope | Files touched | Risk |
|---|---|---|---|
| **1** | `World` rename, `Entity`/`Character` shape, cast removal | ~10 | Low — behaviour-neutral by design |
| **2** | 48 content files, spawn channel, dormancy, TileMap, GameManager | ~60 | Medium — most new code, but little of it conflicts |
| **3** | Player, CoinBlock, view, tuning, cleanup | ~20 | Medium — most hand-merging lives here |

**Rollback:** each task is a separate commit on `integration/merge-tmp-feat`. `tmp` and `feat` are
never modified, so `git reset --hard` to the previous checkpoint is always safe.

**Await instruction before starting each task.**
