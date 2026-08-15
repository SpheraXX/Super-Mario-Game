Notes: This instance of Claude is an independent reviewer of both branches, and is therefore slightly less knowledgable of, but also less biased towards any sides in the project.

# Conflict Severity Report
> Branches: `39bcf13` (yours, k.2.6) vs `7e9ae6e` (teammate, "Added coin block appear effect")
> Common ancestor: `c74283c` ("feat: add new entities and player state movements")
> Files with two-way edits: **31**

---

## Executive Summary

The two branches are **not a simple linear divergence** — they represent two fundamentally different
architectural directions taken from the same base. Your branch (`k.2.x`) deeply refactored the
Controller layer (extracted `LevelScene`, `LevelClearSequence`, `PortalSystem`), redesigned physics
(`getJumpForce` → `getMaxJumpSpeed + getJumpAccel`), and expanded `GameManager`. The teammate's
branch added new enemy types, a projectile system, item spawning, a `World` interface, dormancy,
and crouching. **Most conflicts are not line-level clashes — they are design-level incompatibilities.**

---

## 🔴 CRITICAL — Fundamentally Incompatible Designs (requires a design decision before merge)

### `include/Model/Entity.h`
| Side | What changed |
|------|-------------|
| **Yours** | Stripped `Entity` down aggressively: removed `handleInput`, `takeDamage`, `onStomped`, `onHit`, `getDamageValue`, `isAlive`, `isDying`, `velocity`, `isGrounded`, `getVelocity/setVelocity` from the base class. Added `onTriggerEnter`. |
| **Theirs** | Kept all of the above. **Added** `usesTileCollision()`, `isStompable()`, `canBreakBricks()`, `drawsBehindTerrain()`, `setWorld(World*)` and a protected `world` ptr. Also added `isDormant` field. |

**Conflict:** The two sides have **opposite philosophies** for what `Entity` should be. Yours strips the base class (good OOP); theirs keeps the wide interface but adds more virtual queries (more pragmatic for polymorphic dispatch). These cannot be auto-merged — every consumer of `Entity` in both branches will break until you decide which shape the base takes.

---

### `include/Model/Character.h`
| Side | What changed |
|------|-------------|
| **Yours** | Moved `velocity` and `isGrounded` into `Character` (out of `Entity`). Renamed `getJumpForce` → `getMaxJumpSpeed` + `getJumpAccel`. Added `setWorld(World&)`, world physics getters, promoted `handleInput(float dt)` as a virtual. Renamed gravity constants (`Gravity` → `DefaultGravity`, etc.). |
| **Theirs** | Kept `velocity` on `Entity`. Kept `getJumpForce`. Added `AnimState::Crouch`. Added `getGravityScale()/setGravityScale()`. Removed `setMap()` and `resolveTileCollisions()`. |

**Conflict:** Both sides modify the same block of physics-tuning virtual functions, rename the same constants, and disagree on where `velocity` lives. The jump API is entirely different. Cannot auto-merge.

---

### `include/Model/Player/Player.h` + `src/Model/Player/Player.cpp`
| Side | What changed |
|------|-------------|
| **Yours** | Reworked `handleInput(float dt)` to use acceleration/inertia (smooth movement). Added coyote time, jump buffering, `inputMoving`, `inputDown`. Moved `score/coins` to `GameManager` only. Added many `constexpr` physics constants to the header. |
| **Theirs** | Added `crouching`, `syncPowerSize()`, `isBig()`, `isFire()`, `isStar()`, `isLuigi()`, `canBreakBricks()`, `getBlinkRemaining()`, `horizontalInput`, `sprinting` members. Kept `score/coins` on Player. The fireball shooting logic (`MarioFireball`) is added here. |

**Conflict:** The `handleInput` function body is **completely different** in both — yours uses acceleration/inertia, theirs uses instant velocity snapping + crouch. Both also have different members in the protected section that touch the same logical fields (`sprinting`, `score`, `coins`). Deeply incompatible at the logic level.

---

### `include/Controller/PlayState.h`
| Side | What changed |
|------|-------------|
| **Yours** | Refactored `PlayState` completely: extracted everything into `LevelScene` + `LevelClearSequence`. PlayState now only owns those two + `HudRenderer`. Removed all entity/collision/renderer fields. |
| **Theirs** | `PlayState` **inherits from `model::World`** and implements `spawn()` + `getPlayer()`. Added camera management (`cameraX`, `activationFrontier`), dormancy arming, `pendingEntities`, `playAsLuigi`. |

**Conflict:** These are architecturally divergent. Yours deleted what theirs extended. Cannot be merged without a clear decision about whether `PlayState` is the `World` or delegates to a `LevelScene`.

---

### `include/Model/Core/GameManager.h` + `src/Model/Core/GameManager.cpp`
| Side | What changed |
|------|-------------|
| **Yours** | Added: `coins`, `levelClearBonus`, `currentMapPath`, `nextMapPath`, `levelName`, plus all their getters/setters and `CoinsPerLife` / `DefaultMapPath`. |
| **Theirs** | Added: `coins` + `addCoin(int count=1)`. Added an **embedded LevelTimer** inside `GameManager` (`startLevelTimer`, `tickTimer`, `getTimeRemaining`, `isTimeUp`, `awardTimeBonus`, `timeRemaining`). `CoinsPerExtraLife = 50` (yours uses `CoinsPerLife = 100`). |

**Conflict:** Both add `coins` (same field name, different implementations). Constant value clash (`50` vs `100`). Yours puts the timer in a separate `LevelTimer` class; theirs embeds it in `GameManager`. The `addCoin` signature differs (`void addCoin()` vs `void addCoin(int count=1)`).

---

### `src/Model/Block/CoinBlock.cpp`
| Side | What changed |
|------|-------------|
| **Yours** | Changed trigger to `onBlockHit(BlockHitEvent&)`. Added animation state (`coinPopElapsed`). Added trace logging. Calls `addCoin()`. |
| **Theirs** | Kept `onCollision` trigger. Added random item spawning (Mushroom/FireFlower/Starman/Coin) via `world->spawn()`. Added overlap epsilon check. Uses `dynamic_cast<Player*>`. |

**Conflict:** The entire collision-handling function body is different — different trigger method, different reward logic, different API calls. Also `CoinBlock.h` differs: yours adds `isOpened/isCoinPopping/getCoinPopProgress`; theirs adds reward-chance constants.

---

## 🟠 HIGH — Significant Incompatibilities (logic-level changes on same functions)

### `include/Model/Enemy/Enemy.h` + `src/Model/Enemy/Enemy.cpp`
| Side | What changed |
|------|-------------|
| **Yours** | Removed `override` from `onHit` / `getDamageValue` (these no longer live on `Entity`). Minor comment changes. |
| **Theirs** | Added `createProjectile()` Factory Method, `getScoreValue()`, `awardScore()`, `updateAttack()`, `findPlayer()`, attack cooldown fields, `Projectile` forward declaration. |

**Risk:** Yours removed `override` because `Entity` no longer has those methods; theirs kept them as overrides. This is a compile-time clash depending on which `Entity.h` wins.

---

### `include/Model/Player/Mario.h` + `Luigi.h`
| Side | What changed |
|------|-------------|
| **Yours** | Renamed `getJumpForce()` → `getMaxJumpSpeed()` + `getJumpAccel()`. Changed `RunSpeed` from `400` → `360`. |
| **Theirs** | Added `isLuigi() const override`. Kept `getJumpForce()` and `RunSpeed = 400`. |

**Risk:** The virtual function name changed on your side; theirs still overrides the old name. Compile error guaranteed if both sides are naively merged.

---

### `include/Model/Map/TileMap.h` + `src/Model/Map/TileMap.cpp`
| Side | What changed |
|------|-------------|
| **Yours** | Added castle-tile symbol array, `loadFromLines()`, `padRight()`, `setTile()`, metadata header (`levelName`, `worldType`, `nextMapPath`) and `parseHeader()`. |
| **Theirs** | Added `SpawnPoint` struct, `BrickSymbol`/`CoinBlockSymbol` constants, `breakTile()`, `getSpawnPoints()`, `getCoinBlockSpawns()`, `tileOrigin()`. Added `spawnPoints` and `coinBlockSpawns` fields. |

**Risk:** Both add new fields to `TileMap`. No field names clash, but `isCastleSymbol` vs `BrickSymbol`/`CoinBlockSymbol` may conflict if the renderer's symbol table overlaps. Semantically mergeable but requires careful review of the symbol namespace.

---

### `src/Model/Core/CollisionManager.cpp`
| Side | What changed |
|------|-------------|
| **Yours** | Completely rewrote it (~392 added lines from ancestor). This is the full refactored collision system from `k.2.x`. |
| **Theirs** | Also heavily modified (~392 lines diff): added dormancy checks, `usesTileCollision()` gate, `isStompable()` routing, `canBreakBricks()` brick smashing, `breakTile()` calls, Coin spawn on brick break, `setWorld()` call. |

**Risk:** Both touched essentially the entire file, but in different ways. Your version reflects the new architecture (LevelScene etc.); theirs reflects new entity-type queries on Entity. The exact diff intersection is massive.

---

## 🟡 MEDIUM — Parallel additions (likely auto-mergeable with review)

| File | Your change | Their change | Risk |
|------|-------------|--------------|------|
| `src/Model/Enemy/Goomba.cpp` | Adjusted walk/turn logic | Added scoring + `awardScore()` | Low overlap |
| `src/Model/Enemy/Koopa.cpp` | Shell state tweaks | Added scoring + projectile hooks | Low overlap |
| `src/View/Enemy/GoombaRenderer.cpp` | Minor frame/animation changes | Same | Likely clean |
| `src/View/Enemy/KoopaRenderer.cpp` | Shell animation | Shell + new state frames | Moderate |
| `src/View/Player/PlayerRenderer.cpp` | Heavily reworked (power states, animation) | Added Luigi row, crouch frame, blink | Both are large rewrites — conflict likely |
| `include/View/HudRenderer.h` + `src/View/HudRenderer.cpp` | Reworked to use `HudData` snapshot struct | Teammate also modified HUD (timer display) | Depends on exact overlap |
| `src/View/Map/TileMapRenderer.cpp` | Castle tile registration | Coin block + brick rendering | Symbol table may clash |
| `assets/maps/debug.map` | New map layout | Different layout | Binary merge of ASCII — review manually |

---

## 🟢 LOW / CLEAN — Only one side touched (no true conflict)

These files appear in the overlap list but effectively only one side made substantive changes from the ancestor:

- `src/Model/Player/Luigi.cpp` / `Mario.cpp` — yours renamed the jump function; theirs added `isLuigi()`. Different lines, likely auto-mergeable.
- `src/Model/Core/GameManager.cpp` — both added new methods but to different sections.
- `src/Model/Entity.cpp` — theirs adds body for `usesTileCollision`; yours may be empty diff.

---

## Files ONLY In Your Branch (no conflict — safe)
`LevelScene`, `LevelCompletion`, `LevelClearSequence`, `PortalSystem`, `LevelGeometry`, `AppEngine` refactor, `World/WorldSet/WorldType`, `Level`, `FlagPole`, `Pipe`, `Block`, `BrickBlock`, `AssetManager`, `SpritePainter`, `TextUtils`, `HudData`, `RenderContext`, `BrickBlockRenderer`, `FlagPoleRenderer`, `PipeRenderer`, and all associated `.cpp` files.

## Files ONLY In Their Branch (no conflict — safe)
`Bowser`, `HammerBro`, `Lakitu`, `PiranhaPlant`, `Spiny`, `EnemyFactory`, `Item`, `Mushroom`, `FireFlower`, `Starman`, `Coin`, `Projectile`, `MarioFireball`, `Fireball`, `Hammer`, `SpinyEgg`, `AtlasFrameRenderer`, `EnemyAtlas`, `ItemAtlas`, `ItemFrameRenderer`, `FireballRenderer`, `CMakeLists.txt`, `World.h` (their new model::World interface).

---

## Summary Table

| Severity | Count | Files |
|----------|-------|-------|
| 🔴 Critical | 6 | `Entity.h`, `Character.h`, `Player.h/.cpp`, `PlayState.h`, `GameManager.h/.cpp`, `CoinBlock.cpp` |
| 🟠 High | 4 | `Enemy.h/.cpp`, `Mario.h`+`Luigi.h`, `TileMap.h/.cpp`, `CollisionManager.cpp` |
| 🟡 Medium | 8 | Renderer files, Goomba/Koopa .cpp, HudRenderer, TileMapRenderer, debug.map |
| 🟢 Low | ~13 | Single-side functional additions that happened to touch shared files |
