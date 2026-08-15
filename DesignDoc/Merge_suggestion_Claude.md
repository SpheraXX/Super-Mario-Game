Notes: This instance of Claude is an independent reviewer of both branches, and is therefore slightly less knowledgable of, but also less biased towards any sides in the project.

# Merge Decisions
> Based on: `conflict_severity_report.md` and full file reads of both branch tips.
> Format: each section is self-contained so it can be handed to a dev independently.
> **YOUR branch** = `39bcf13` (k.2.6) · **THEIR branch** = `7e9ae6e` (coin block appear effect)

---

## Section 1 — `include/Model/Entity.h`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| `handleInput / takeDamage / onStomped / onHit / getDamageValue` | ❌ Removed from base | ✅ Kept (used by new enemies via override) |
| `isAlive / isDying` | ❌ Removed from base | ✅ Kept |
| `velocity / isGrounded` | ❌ Moved to `Character` | ✅ Kept on `Entity` |
| `onTriggerEnter` | ✅ Added | — |
| `usesTileCollision / isStompable / canBreakBricks / drawsBehindTerrain` | — | ✅ Added (needed for new enemies) |
| `setWorld(World*) / world ptr` | — | ✅ Added (needed for spawn system) |
| `isDormant` | — | ✅ Added (camera-based activation) |

### Decisions

**`velocity` and `isGrounded` → Keep on `Entity` (THEIRS wins)**
Reason: their new `Item`, `Projectile`, `Coin`, `Fireball` classes all need velocity and ground state but are **not** Characters. Moving those to `Character` breaks the entire item/projectile layer. This overrides your refactor in this one spot.

**`handleInput / takeDamage / onStomped / onHit / getDamageValue / isAlive / isDying` → REMOVE from `Entity` (YOURS wins)**
Reason: having no-op defaults for damage and stomping on a `Castle` or `Pipe` is an ISP violation — your critique from the OOP review was correct. These belong on `Character` and `Enemy` respectively, where they already have real implementations. The new enemies work through `Character`/`Enemy` polymorphism, not through `Entity` directly.

**`onTriggerEnter` → KEEP (YOURS)**
Clean hook for `FlagPole`. No conflict with theirs.

**`usesTileCollision / isStompable / canBreakBricks / drawsBehindTerrain` → KEEP ALL (THEIRS)**
All four are needed by the new enemy types (`Lakitu`, `PiranhaPlant`, `Spiny`, `Bowser`). They are additive and don't conflict with your stripped base.

**`setWorld(World*) / protected world ptr` → KEEP (THEIRS)**
Needed by `CoinBlock`, `Enemy`, and `Projectile` to call `world->spawn()`. Compatible with your `World` interface class from `include/Model/Core/World.h`.

**`isDormant` → KEEP (THEIRS)**
Needed by the camera-activation system. Clean boolean, no side effects on the rest of the interface.

### Resolved shape
```
Entity owns: position, size, hitbox, velocity, isActive, isGrounded, isDormant, world*
Entity virtuals: update, onCollision, onTileCollision, onTriggerEnter,
                 isSolid, isAlive, isDying,
                 usesTileCollision, isStompable, canBreakBricks, drawsBehindTerrain
Entity NOT: handleInput, takeDamage, onStomped, onHit, getDamageValue  ← these go on Character/Enemy
```

---

## Section 2 — `include/Model/Character.h`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| `velocity` | Moved here from Entity | Stays on Entity |
| Jump API | `getMaxJumpSpeed() + getJumpAccel()` | `getJumpForce()` (single value) |
| Gravity constants | Renamed `DefaultGravity / DefaultMaxFallSpeed` | `Gravity / MaxFallSpeed` (original names) |
| `setWorld / getGravity / getMaxFallSpeed / isUnderwater` | ✅ Added (World-based physics) | — |
| `gravityScale` | — | ✅ Added (per-character multiplier) |
| `AnimState::Crouch` | — | ✅ Added |
| `handleInput(float dt)` | ✅ Here as virtual | On Entity (old signature) |
| `setMap / resolveTileCollisions` | `setMap` kept | Both removed |
| `deathElapsed` | Removed | Kept |
| `worldPtr` | `const World*` | — |

### Decisions

**`velocity` → stays on `Entity` (Section 1 decision cascades)**
`Character` no longer re-declares it; it inherits it from `Entity`.

**Jump API → `getMaxJumpSpeed() + getJumpAccel()` (YOURS wins)**
The two-parameter model is physically correct: it separates the ceiling from the acceleration, enabling the tuned "barely 4-tile" jump arc described in Mario.h. `getJumpForce()` as a single impulse is simpler but can't do variable-hold jumps. Since the full jump physics (coyote, buffer, hold-time) already live in Player, the interface should match.

**Gravity constants → use YOUR rename (`DefaultGravity / DefaultMaxFallSpeed`)**
Makes it unambiguous that these are the bare-land baseline, not the only values in the system.

**`setWorld / getGravity / getMaxFallSpeed / isUnderwater` → KEEP (YOURS)**
Elegant: the physics scaling lives in `World`, Characters read it through these wrappers. Required for the underwater world type.

**`gravityScale` → KEEP (THEIRS)**
Critical for `Lakitu` (hover), `PiranhaPlant` (fixed), `Fireball` (lazy arc), `Spiny` (lobbed egg arc). Coexists with world-based scaling: final gravity = `DefaultGravity * worldScale * gravityScale`.

**`AnimState::Crouch` → KEEP (THEIRS)**
Required by the crouching player animation. Zero cost to keep.

**`handleInput(float deltaTime)` → KEEP YOUR signature (virtual on Character)**
`dt` is needed for acceleration math. Default is a no-op, so non-player characters are unaffected.

**`deathElapsed` → KEEP (THEIRS)**
Your version removed it; needed for the death-animation timer. Add it back.

**`setMap` → KEEP (YOURS)**
Theirs removed it, but tile collision resolution still needs the map pointer.

---

## Section 3 — `include/Model/Player/Player.h` + `src/Model/Player/Player.cpp`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| Movement model | Acceleration / inertia (GroundAccel, AirAccel, Friction) | Instant velocity snap |
| Coyote time + jump buffer | ✅ Added | — |
| Crouching | — | ✅ Added (`crouching`, `syncPowerSize`, `CrouchHeight`) |
| `score / coins` fields | Removed (GameManager only) | Kept on Player |
| `isFire / isStar / isBig` | — | ✅ Added (renderer queries) |
| `isLuigi()` | — | ✅ Added (renderer row selection) |
| `canBreakBricks()` | — | ✅ Added |
| Blink invulnerability | — | ✅ Added (`DamageBlinkTime`, `getBlinkRemaining`) |
| `horizontalInput / sprinting / animationClock` | — | ✅ Added |
| `handleInput` signature | `handleInput(float dt)` | `handleInput()` (no dt) |
| Trace logging | ✅ Added (temp, in .cpp) | — |

### Decisions

**Movement model → YOURS (inertia/acceleration)**
The acceleration model is objectively better for a platformer: it eliminates the teleporting-stop feel, matches the actual SMB physics, and the coyote/buffer forgiveness windows are essential for tight platforming. The jump-hold logic also depends on per-frame `dt`.

**Crouching system → KEEP (THEIRS)**
`syncPowerSize()`, `crouching`, `CrouchHeight`, `SmallHeight`, `BigHeight` — all needed and well-designed. Integrates cleanly with the inertia model (crouch locks `horizontalInput = 0`).

**`score / coins` → REMOVE from Player (YOURS wins)**
The GameManager comment in their own code says it best: *"the player entity is destroyed and rebuilt on every death — score and coins have to outlive it."* Keeping them on Player contradicts their own reasoning. Player keeps the thin wrappers (`addScore`, `getScore`, etc.) that forward to GameManager.

**`isFire / isStar / isBig / isLuigi / canBreakBricks / getBlinkRemaining` → KEEP ALL (THEIRS)**
These are clean read-only queries needed by renderers and the collision system. None of them involve type-checking (`isBig()` reads size, not concrete type). `isLuigi()` avoids `dynamic_cast` in the renderer — correct.

**`horizontalInput / sprinting / animationClock` → KEEP (THEIRS)**
`horizontalInput` is needed to detect wall-push for animation; `animationClock` drives the walk frame cycle. Both integrate into the inertia model without conflict.

**`handleInput` signature → YOURS (`float dt`)**
Required for the acceleration math.

**Trace logging in `.cpp` → REMOVE**
The `trace_log.txt` is already untracked (seen in `git status`) and is a debug artifact. Remove the `#include <fstream>`, the anonymous `trace()` function, and every `trace(...)` call in CoinBlock.cpp and Player.cpp.

**`DamageBlinkTime = 2.0f` vs `DamageCooldownTime = 0.5f`**
These are different things: yours is the invulnerability window, theirs is the blink duration. Keep **both** — `DamageCooldownTime` for the actual invincibility frames, `DamageBlinkTime` for the visual blink which can be longer.

---

## Section 4 — `include/Controller/PlayState.h` (+ LevelScene / World)

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| Architecture | Extracted `LevelScene` + `LevelClearSequence` + `PortalSystem` | Kept monolithic PlayState |
| `model::World` impl | `World` is a separate interface in `include/Model/Core/World.h` | `PlayState` inherits `model::World` directly |
| Camera + dormancy | In `LevelScene` (inferred) | In `PlayState` (`cameraX`, `activationFrontier`, `armDormancy`) |
| `pendingEntities` | In `LevelScene` | In `PlayState` |
| `playAsLuigi` toggle | — | In `PlayState` |

### Decisions

**Architecture → YOURS (LevelScene extraction)**
PlayState as a God Object was identified as the primary design flaw. The extraction is the right call. PlayState should remain thin: it owns a `LevelScene`, a `LevelClearSequence`, a `HudRenderer`, and a `HudData` — nothing else.

**`model::World` implementor → LevelScene, NOT PlayState (SYNTHESIS)**
Their approach of `PlayState : public model::World` puts a controller class inside the model layer's abstraction boundary — a layer violation. The correct home for `World` implementation is `LevelScene`, which already owns entities, the camera, and `pendingEntities`. `LevelScene` becomes the concrete `World` that entities talk to.

**Camera + dormancy + `pendingEntities` → Move into `LevelScene` (from THEIRS)**
These are level-runtime concerns that belong in the scene object, not in a state-machine node. `LevelScene::update()` handles camera scroll, `armDormancy()`, and the pending-entity splice. `PlayState::update()` just calls `scene->update(dt)`.

**`playAsLuigi` toggle → Keep in `PlayState`**
It's a controller-level input toggle (C key), not a scene concern. PlayState passes the character choice when it constructs the scene.

---

## Section 5 — `include/Model/Core/GameManager.h` + `.cpp`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| `coins` field | ✅ Added | ✅ Added (same name, same type) |
| `addCoin` signature | `void addCoin()` | `void addCoin(int count = 1)` |
| Coin-per-life constant | `CoinsPerLife = 100` | `CoinsPerExtraLife = 50` |
| Map path / level name fields | ✅ Added | — |
| `levelClearBonus` | ✅ Added | — |
| Embedded LevelTimer | — | ✅ Added (`startLevelTimer / tickTimer / getTimeRemaining / isTimeUp / awardTimeBonus`) |
| Separate `LevelTimer` class | ✅ Already exists in your branch | — |

### Decisions

**`coins` field → trivially merge (same on both sides)**

**`addCoin` signature → THEIRS (`int count = 1`)**
More flexible; default arg means all existing `addCoin()` call sites compile unchanged.

**Coin-per-life constant → YOURS (`CoinsPerLife = 100`)**
100 is canonical SMB. Their 50 was justified by "this clone is shorter" — that's a tuning decision, not an architectural one, and 100 is the expected value.

**Map path / level name / `levelClearBonus` → YOURS**
Required by `LevelCompleteState` and the map-chaining system. Theirs has none of this.

**Embedded LevelTimer → REJECT (YOURS wins, use separate class)**
The embedded timer in GameManager swells it further as a God Object. You already have a proper `LevelTimer` class in your branch with `update/pause/resume/reset`. Keep that. `PlayState` / `LevelScene` owns and drives the `LevelTimer`; GameManager is not a timer.

---

## Section 6 — `include/Model/Block/CoinBlock.h` + `.cpp`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| Collision trigger | `onBlockHit(BlockHitEvent&)` | `onCollision(Entity&, CollisionType)` |
| Animation API | `isOpened / isCoinPopping / getCoinPopProgress` | — |
| Reward system | Always a coin | Random: Mushroom / FireFlower / Starman / Coin |
| Spawning mechanism | `GameManager::addCoin()` direct | `world->spawn(...)` |
| Overlap epsilon check | — | ✅ Added |
| `dynamic_cast<Player*>` | — | ✅ Used to check size for mushroom direction |
| Trace logging | ✅ Present (temp) | — |

### Decisions

**Collision trigger → `onBlockHit` (YOURS)**
A dedicated event hook is cleaner than overriding `onCollision` and re-checking the side. `BlockHitEvent` already carries the bumper reference and side; no need to re-derive it inside the block.

**Animation API → YOURS (`isOpened / isCoinPopping / getCoinPopProgress`)**
The CoinBlockRenderer needs these. Keep the `coinPopElapsed` timer.

**Random item spawning → KEEP (THEIRS)**
This is the main feature of their commit. The probability table (30% mushroom, 15% flower, 15% star, 40% coin) is a gameplay decision to accept. Use `world->spawn()` inside `onBlockHit`, not inside `onCollision`.

**`dynamic_cast<Player*>` for mushroom direction → REPLACE**
This is explicit type-checking — the OOP flaw we want to eliminate. Replace with: `BlockHitEvent` already holds a reference to the bumper entity; add `getDirection()` to `Entity` (or better, check it through the existing `Entity::getDirection()` which is already on `Character`). Since bumping requires a `Character`, cast to `Character*` (still a downcast but at least to an abstract supertype, not a concrete leaf class). Actually cleanest: expose `getDirection()` on `Entity` (it's already meaningful — anything with velocity has a direction).

**Overlap epsilon check → KEEP (THEIRS)**
Good correctness fix. Independent of the other decisions.

**Trace logging → REMOVE**
Same decision as Section 3.

---

## Section 7 — `include/Model/Enemy/Enemy.h` + `.cpp`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| `onHit / getDamageValue` | No `override` keyword | `override` |
| `createProjectile()` Factory Method | — | ✅ Added |
| `getScoreValue()` | — | ✅ Added |
| `awardScore()` | — | ✅ Added |
| `updateAttack(dt)` | — | ✅ Added |
| `findPlayer()` | — | ✅ Added |
| `attackCooldown / attackTimer` fields | — | ✅ Added |

### Decisions

**All theirs' additions → KEEP ALL**
`createProjectile` is a clean Factory Method (exactly what the OOP review recommended for enemy attacks). `awardScore` centralizes kill credit. `findPlayer` queries through `world->getPlayer()` — no type-checking. These are all additive with no conflict.

**`override` keyword → depends on Section 1 decision**
Since we're removing `onHit` and `getDamageValue` from `Entity`, the `override` keyword on `Enemy` was correct in theirs (they had those on Entity). After the merge, `onHit` and `getDamageValue` will be declared first in `Enemy`, so remove `override` there — consistent with yours.

---

## Section 8 — `include/Model/Map/TileMap.h` + `.cpp`

### What each side did
| Item | Yours | Theirs |
|------|-------|--------|
| Castle symbols / `isCastleSymbol` | ✅ Added | — |
| `loadFromLines / padRight / setTile` | ✅ Added | — |
| Metadata (`levelName`, `worldType`, `nextMapPath`) | ✅ Added | — |
| `SpawnPoint` struct | — | ✅ Added |
| `BrickSymbol / CoinBlockSymbol` constants | — | ✅ Added |
| `breakTile()` | — | ✅ Added |
| `getSpawnPoints / getCoinBlockSpawns` | — | ✅ Added |
| `tileOrigin()` | — | ✅ Added |

### Decisions

**All items → MERGE BOTH SIDES**
These are genuinely additive — no field name clashes, no method signature collisions.

**Symbol namespace check**
- `'B'` (theirs, BrickSymbol): NOT in your castle symbol list `{A,D,F,H,I,J,L,N,Q,R,S,U,V,W,X,Y,Z,a,b,c,d}` ✅ safe
- `'C'` (theirs, CoinBlockSymbol): matches your existing usage (`Block(position, size, 'C')`) ✅ already consistent

**One ordering note**: `SpawnPoint` struct should be declared before `class TileMap` in the header (it's a dependency of TileMap's return types). Both sides already do this correctly on their respective versions.

---

## Priority Order for Implementation

Once you're ready to merge, tackle sections in this order (each unlocks the next):

```
1. Entity.h          ← everything depends on this shape
2. Character.h       ← depends on Entity
3. TileMap.h/.cpp    ← independent, unlocks PlayState/LevelScene
4. GameManager.h/.cpp ← independent service layer
5. Enemy.h/.cpp      ← depends on Entity + Character being settled
6. Player.h/.cpp     ← depends on Character + GameManager
7. CoinBlock.h/.cpp  ← depends on Entity (world ptr) + Player (direction)
8. PlayState/LevelScene ← integrates everything above
```

---

## Open Questions for You

> [!IMPORTANT]
> **Q1 — `Mario::RunSpeed`**: Yours changed `400 → 360`; theirs kept `400`. Which feel do you want?
> `360` is the tuned value with drag commentary; `400` is the original. This is a pure gameplay call.

> [!IMPORTANT]
> **Q2 — `FireCooldownDuration`**: Yours has `0.5s`; theirs has `1.0s`. Which feels right?

> [!IMPORTANT]
> **Q3 — `DamageCooldownTime` vs `DamageBlinkTime`**: Do you want a short invulnerability window (0.5s, yours) that the player can clearly see has ended, or the longer blink duration (2.0s, theirs)? These can be kept as separate constants for separate purposes, or unified.

> [!NOTE]
> **Q4 — `CoinBlock` random rewards**: Theirs has 40% coin / 30% mushroom / 15% flower / 15% star. That's a 60% chance of a power-up from every single `?` block, which feels very high for a Mario game. Worth adjusting the probabilities before committing.
