# Merge Progress

Resumable state for the `tmp` × `feat` integration. Plan: `MERGE_TASKS.md`. Decisions:
`MERGE_PLAN.md` §4.

| | |
|---|---|
| Branch | `integration/merge-tmp-feat` (branched from `tmp` @ `39bcf13`) |
| Last updated | 2026-08-11 |
| **Status** | **Task 1 complete — awaiting human play-test sign-off before Task 2** |

## Commits so far

```
32d7009  Task 1: World -> WorldTheme rename, Entity life-state virtuals, cast removal
d4ae041  docs: merge analysis, decisions and task plan for tmp x feat integration
39bcf13  k.2.6                                            <- tmp, branch point
```

Rollback to any checkpoint: `git reset --hard <sha>`. `tmp` and `feat` are untouched.

---

## ✅ Task 1 — complete

| Step | Done |
|---|---|
| 1.1 Integration branch from `tmp` | ✅ |
| 1.2 D-4 `World` → `WorldTheme` | ✅ |
| 1.3 D-1 Option C — `Entity`/`Character` shape | ✅ |
| 1.4 Remove pass-2 `dynamic_cast`s | ✅ |

**Files changed:** `CMakeLists.txt`, `Entity.h`, `Character.h/.cpp`, `WorldTheme.h/.cpp` (renamed
from `World.*`), `WorldSet.h/.cpp`, `LevelScene.cpp`, `CollisionManager.cpp`.

### Checkpoint 1 — automated checks: ALL PASS
- Clean reconfigure + full rebuild: **0 errors, 0 warnings**
- No stale `model::World` theme references — only `WorldTheme`
- `dynamic_cast` count in `CollisionManager.cpp`: **exactly 2** (pass-1 `Character*` @88,
  pre-existing `Block*` @351)
- No `velocity` / `isGrounded` declared on `Entity`
- Executable launches and runs stably (verified >60s, clean exit on terminate)

### ⚠️ Checkpoint 1 — NOT yet verified (requires a human at the keyboard)
These need someone to actually play. **Task 2 should not start until these are confirmed:**
- [ ] Mario walks / runs / jumps
- [ ] **Jump arc: clears a 4-tile wall, fails a 5-tile wall** ← the critical D-2 regression
- [ ] Stomping a Goomba kills it and bounces the player
- [ ] `G` → death animation, life lost, restart
- [ ] `H` → hitbox overlay toggles
- [ ] Flagpole → clear sequence → level-complete overlay
- [ ] Pipe entry (hold Down on a pipe) teleports

Task 1 is behaviour-neutral by design, so any failure here is a real regression from the rename or
the `isDying()` change — not expected drift.

### Deviations from the plan
1. **`CONFIGURE_DEPENDS` pulled forward from Task 3.5 to Task 1.** The stale-glob failure hit
   during the rename (`fatal error: src\Model\World\World.cpp: No such file or directory`) and
   Task 2 adds ~48 files, where it would hit repeatedly. `CMakeLists.txt` now matches `feat`'s.
2. **`Entity.h` class comment rewritten.** The original said a FlagPole "can never be asked for a
   velocity **or a death state**." Adding `isDying()` made the second half false. The comment now
   documents motion staying on `Character` and explains why life state is the deliberate exception.

---

## ⬜ Task 2 — not started

Content layer: our 48 files + the spawn channel. See `MERGE_TASKS.md` § TASK 2.

**First steps when resuming:**
1. `git checkout integration/merge-tmp-feat`
2. Copy `include/Model/Core/World.h` from `feat` (the spawn interface — the name is now free)
3. Add `setWorld(World*)` + protected `world` ptr to `Entity`
4. `LevelScene : public model::World` — `spawn()` → `pendingEntities`, post-update splice

**Source for every ported file:**
`git show feat/adding-new-entities-state-movements:<path>`

---

## ⬜ Task 3 — not started

Player, CoinBlock, view union, tuning (D-5…D-9), cleanup. See `MERGE_TASKS.md` § TASK 3.

Carry-over items already identified:
- Remove 28 `trace()` calls across 7 files
- Fix `case Key::H` fall-through in `feat`'s `PlayState::handleEvent` (missing `break`) — note this
  bug lives in `feat`'s `PlayState`, which is **not** the one being kept; verify it did not travel
  into the merged `LevelScene`/`PlayState`
- `git rm --cached CMakeUserPresets.json`

---

## Build command

```sh
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:/SFML/SFML-3.0.2" -DSFML_STATIC_LIBRARIES=ON
cmake --build build -j 8
build/bin/SuperMario.exe
```

`C:\mingw64\bin` must be on PATH (it is, on this machine's User PATH).
