# Merge Progress

Resumable state for the `tmp` × `feat` integration. Plan: `MERGE_TASKS.md`. Decisions:
`MERGE_PLAN.md` §4.

| | |
|---|---|
| Branch | `integration/merge-tmp-feat` (branched from `tmp` @ `39bcf13`) |
| Last updated | 2026-08-14 |
| **Status** | **All 3 tasks complete + stomp follow-up + map-format unification (Stage 1, uncommitted) — awaiting human play-test sign-off** |

## Commits

```
4f1dc2d  feat: per-character stomp bounce tuning
90aea68  fix: restore stomp bounce, isStompable routing and star contact
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

## ✅ Follow-up — stomp routing restored (90aea68, 4f1dc2d)
`MERGE_PLAN.md` §5.2 listed `isStompable()` routing among the merge tasks; it and the stomp
bounce were dropped in the resolution and went unnoticed until the play test. `90aea68`
re-applies all three behaviors in `resolveEntityInteraction`:
- **Bounce** — landing on a stompable enemy squashes it and relaunches the player. The bounce
  is a rebound, not the fixed `-350` kick: `bounceUp = max(0, ratio * fallSpeed - constant)`
  with `fallSpeed` = downward speed at impact. The `max(0, …)` floor absorbs slow drops; a
  hard fall throws the player right back up. Horizontal momentum is kept.
- **`isStompable()` gate** — only stompable enemies are squashed from above; landing on
  Spiny / Bowser / Piranha Plant now damages the player again instead of squashing them.
- **Star contact** — a starred player defeats any enemy on contact (checked before the
  stomp lockout, which is skipped for star hits).
`4f1dc2d` moves the two tuneables onto `Player` (`getStompBounceRatio` /
`getStompBounceConstant`, polymorphic like `getWalkSpeed`): stock defaults live on `Player`
(0.85 / 30), Mario keeps them explicitly, Luigi springs higher (0.9 / 20). The
`dynamic_cast` checkpoint count is unchanged (2); the player cast uses the documented
layer contract instead.

## ✅ Follow-up 2 — map-format unification (fixes the Koopa sink; working tree, uncommitted)
The debug maps marked enemies with letters (`'E'` Goomba, `'K'` Koopa) while the feat maps
used digits (EnemyFactory ids) — **two live enemy readers** in `LevelScene::resetLevel`. The
letter path constructed enemies directly with **top-at-cell** placement, so the 23px Koopa
spawned with its feet 7px inside the ground row below the marker; the tile pass's landing
gate (`prevFootY > tileTop + LandingEpsilon`, `CollisionManager`) deliberately refuses to
snap a body already inside a tile (side-brush protection), so the Koopa was never grounded
and sank. The digit path (`EnemyFactory::footAligned`) already handled tall bodies — exactly
why `feat1_1.map`'s digit Koopas were fine and the debug maps' letter Koopas were not.

Resolution (decision: feat's digit format wins, maps converted rather than loader aliases):
- `debug.map` / `debug2.map` / `debug3.map` converted `'E'`→`'0'`, `'K'`→`'1'` (Goomba=0,
  Koopa=1 in `EnemyFactory::Id`). No other symbol differs between the two formats; the
  uncommitted WIP layout of `debug3.map` (2-wide pipe tests) was preserved.
- The `'E'`/`'K'` switch cases were deleted from `LevelScene::resetLevel`; `EnemyFactory`
  is again the only place an enemy is constructed for a level (as its header documents).
- Contract comments updated (`LevelScene.cpp` resetLevel header + digit-loop note,
  `TileMap.h` castle-symbol note): markers are the digits 0-9 in the cell directly above
  the ground; every enemy's feet rest on that marker cell's bottom edge.

Why the Koopa sink is dead — chain for `debug.map`'s `'1'` at grid row 2, col 18:
- Load: digit stripped to `'.'`, `SpawnPoint{1, 2, 18}`.
- Factory: Koopa constructed at the marker cell's top-left `(288, 208)`, then `footAligned`
  drops it by the overhang `23 − 16 = 7` → `(288, 201)`; feet = `224` = ground top exactly.
- Landing gate never trips (feet are flush with the tile top, not inside it); grounding
  works normally every frame.
- The stomp→shell shrink (`Koopa::onStomped`) still re-anchors the feet — consistent with
  the feet-based placement.

Spawn-site audit for the same tall-body hazard (all safe; no change needed):
- `'M'` Mario / fallback Mario / `'C'` / `'#'` / `'B'`: 16px tile-exact bodies — feet land
  exactly on the cell bottom edge.
- `PortalSystem::landingY`: `groundTop - playerHeight` — explicitly size-aware (and the
  pipe-cap variant).
- `PiranhaPlant` (digit 8): documented exception — hangs off the pipe below the marker
  (+16 offset), never stands on the ground.

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
- [ ] **Stomp:** Goomba squash + bounce; bounce height scales with fall height (near-apex trickle ≈ no bounce, full drop relaunches ~2.5 tiles); a held jump key does **not** boost the bounce
- [ ] **Stomp ladder:** Koopa walking → shell; shell → spins + bounce; spinning → idle + bounce
- [ ] **Non-stompables:** landing on Spiny / Bowser / Piranha Plant from above **damages** the player and does not squash them; side/below contact still damages
- [ ] **Star:** touching Goomba / Spiny / Koopa defeats them; Bowser drains health per hit
- [ ] Mario (stock 0.85/30) vs Luigi (0.9/20) bounce feels slightly different
- [ ] Hammer Bro throws; Spiny egg lobbed; Lakitu hovers; Piranha Plant draws behind its pipe
- [ ] Enemies wake on camera approach and do **not** re-arm when backtracking
- [ ] Damage → blink and invulnerability begin and end together
- [ ] `C` switches Mario/Luigi; `H` toggles hitboxes only
- [ ] Flagpole → clear sequence → level-complete overlay
- [ ] Pipe/portal transitions; timer expiry kills; death → life lost → Game Over at zero

**Map-format unification (Follow-up 2):**
- [ ] `debug.map`: the Goomba at col 15 and the Koopa at col 18 stand **flush on the ground** with no sink-in (the reported bug); both overlap the ground line only with the renderer's art buffer
- [ ] Stomp the debug.map Koopa → shell still lands feet-anchored; stomp again → it pops up from the shell spot (never sinks)
- [ ] **No visual difference** in `feat1_1.map` — digit Goombas / Koopas / Paratroopas spawn exactly as before
- [ ] `debug2.map` (underwater carousels) and `debug3.map` (3 areas + portal transitions, including the 2-wide pipe WIP) load and play; enemies flush on their ground rows in every area
- [ ] Blank cells that used to hold `E`/`K` render as empty space (no tile, no ghost sprite)

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
- **Paratroopa patrol anchor** — `Koopa::flyBaseY` is captured in the constructor from the
  marker position, but `EnemyFactory::make<>` repositions the body afterwards (feet-aligned,
  −7px). The patrol arc therefore sits 7px below the intended altitude; a Paratroopa
  (digit `'2'`) marked directly above the ground dips into it at the bottom of its arc
  (visible in `feat1_1.map`). Suggested fix for later: `Koopa::reanchorFlight()` (sets
  `flyBaseY` from the final position), called for winged Koopas in `EnemyFactory::make<>`.

---

## Build

```sh
make
./main.exe   # run from the repo root (runtime DLLs live there)
```

(The repo also carries a CMake setup, but the project builds with the Makefile; the CMake
`build/` directory from earlier smoke-testing is not used anymore.)
