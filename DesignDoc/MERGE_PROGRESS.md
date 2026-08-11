# Merge Progress

Resumable state for the `tmp` × `feat` integration. Plan: `MERGE_TASKS.md`. Decisions:
`MERGE_PLAN.md` §4.

| | |
|---|---|
| Branch | `integration/merge-tmp-feat` (branched from `tmp` @ `39bcf13`) |
| Last updated | 2026-08-11 |
| **Status** | **All 3 tasks complete — awaiting human play-test sign-off** |

## Commits

```
59629d9  Task 3: player abilities, coin-block rewards, tuning and cleanup
2416b90  Task 2: port feat's content layer and the runtime spawn channel
8eb6d50  docs: add resumable merge progress tracker
32d7009  Task 1: World -> WorldTheme rename, Entity life-state virtuals, cast removal
d4ae041  docs: merge analysis, decisions and task plan
39bcf13  k.2.6                                            <- tmp, branch point
```

Rollback: `git reset --hard <sha>`. `tmp` and `feat` are untouched.

---

## ✅ Task 1 — complete
`World` → `WorldTheme` rename (D-4 option A); `isAlive()`/`isDying()` virtuals on `Entity` with
motion staying on `Character` (D-1 option C); both O(n²) pass-2 `dynamic_cast`s removed.
Pulled `CONFIGURE_DEPENDS` forward from Task 3 — the stale-glob failure hit during the rename.

## ✅ Task 2 — complete
48 content files (5 enemies + `EnemyFactory`, 5 projectiles, 5 items, atlas renderers).
`LevelScene` implements `model::World` (D-3) with deferred `pendingEntities` splice.
Dormancy with monotonic frontier; behind-terrain render pass; TileMap `SpawnPoint` parsing;
winged Koopa; `Projectile`/`Item` collision layers; colour-key sprite ctor; `addCoin(int)`.
Map `assets/maps/feat1_1.map` adapted from feat's `level1_1` — now the default.

## ✅ Task 3 — complete
`syncPowerSize()` (feet-anchored resize); fireball firing wired up; `canShoot`/`shoot` hoisted to
`PlayerState` base with Star forwarding **and** ticking the wrapped state; CoinBlock random rewards
via `world->spawn()` behind tmp's `onBlockHit`; D-5/D-7/D-8/D-9 tuning; all 28 `trace()` calls
removed; `CMakeUserPresets.json` untracked.

---

## Checkpoint status — automated: ALL PASS

| Check | Result |
|---|---|
| Clean reconfigure + full rebuild | **0 errors, 0 warnings** |
| `trace(` call sites | **0** |
| `dynamic_cast` in `CollisionManager.cpp` | **2** (pass-1 `Character*`, pre-existing `Block*`) |
| `class World` declarations | **1** (the spawn interface) |
| `WorldTheme` | 1 definition + 1 forward decl |
| `CMakeUserPresets.json` tracked | **no** |
| D-5 `RunSpeed` / D-7 `FireCooldown` / D-9 damage | 360 / 0.5s / 1.0s |
| D-8 reward table | 75 / 15 / 5 / 5 |
| Enemy spawns at runtime | **19 across 7 types**, verified from trace before removal |
| Runtime stability | **6/6 trials ran past 15–20s**, no stderr, no crash |

### On an exit that looked like a regression
Several runtime trials reported `EXITED code=0` shortly after launch. Bisecting to the Task 2
commit reproduced it there too, and pointing the build back at `debug.map` did not. But four
consecutive trials afterwards, and two more on the final build, all ran clean past 15–20s with no
stderr. **Nothing was changed to "fix" it** — the readings were a test-harness artifact (leftover
instances from earlier trials being force-killed, plus PowerShell's `Start-Process` resolving a
relative `-FilePath` against the process cwd rather than `-WorkingDirectory`). Recorded here so the
false alarm is not rediscovered later. If the game ever does exit on its own, note that
`AppEngine::run` loops on `window.isOpen() && !states.empty()`, and nothing in the codebase calls
`popState()` — so a self-exit means a `Closed` event, not a drained state stack.

---

## ⚠️ NOT verified — needs a human at the keyboard

No automated test suite exists (no test target, no framework, on either branch). Everything above
is build verification plus process-level runtime checks. **These still need playing:**

- [ ] **Jump arc: clears a 4-tile wall, fails a 5-tile wall** ← the critical D-2 regression
- [ ] Coyote time; jump buffering
- [ ] Mushroom → Super grows without clipping through the floor; Flower → Fire; Star → invincible
- [ ] Fire Mario shoots with **X**; cooldown ≈ 0.5s; a starred Fire Mario can still shoot
- [ ] `?` block drops all four rewards over ~20 hits, **mostly coins**
- [ ] Big Mario smashes a brick; small Mario bounces
- [ ] Hammer Bro throws; Spiny egg lobbed; Lakitu hovers; Piranha Plant draws behind its pipe
- [ ] Enemies wake on camera approach and do **not** re-arm when backtracking
- [ ] Damage → blink and invulnerability begin and end together
- [ ] `C` switches Mario/Luigi; `H` toggles hitboxes only
- [ ] Flagpole → clear sequence → level-complete overlay
- [ ] Pipe/portal transitions; timer expiry kills; death → life lost → Game Over at zero

---

## Deferred / not done

- **Crouch** (`AnimState::Crouch` enum added, `CrouchHeight` not wired). `syncPowerSize` handles
  Small/Big only. Crouch input and the sitting-pose box remain to be ported from `feat`.
- **`PlayerRenderer` Luigi row / crouch frame / blink fade** — `tmp`'s renderer was heavily
  reworked and was left as-is; `isLuigi()`/`getBlinkRemaining()` exist but the view does not read
  them yet.
- **`Player::isLuigi()` always returns false** — `Luigi` does not override it yet.
- **`dynamic_cast<Player*>` remains in `Mushroom`/`FireFlower`/`Starman`** `onCollect`. This is a
  different case from the CoinBlock one the plan targeted: applying a power-up calls
  `becomeSuper()`/`becomeFire()`/`becomeStar()`, which only `Player` has. The alternative — hoisting
  those onto `Character` — is worse. Left deliberately.
- **`CheepCheep` (id 6)** returns nullptr; needs the water/swimming mechanic.

---

## Build

```sh
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/SFML/SFML-3.0.2" -DSFML_STATIC_LIBRARIES=ON
cmake --build build -j 8
build/bin/SuperMario.exe
```
