# AI Usage Report — Super Mario C++ Project

*A record of one working session with an AI coding assistant, covering gameplay tuning,
rendering/window architecture, asset pipeline work, bug diagnosis and version-control
recovery. Timestamps are taken from the session transcript and are in **UTC**.*

---

## Session Information

| Field | Value |
|---|---|
| **Tool** | Claude Code (CLI / VS Code extension) |
| **Models used** | `claude-opus-5` (primary), `claude-sonnet-5` (two short stretches) |
| **Session span** | 2026-08-13 06:24 UTC → 2026-08-16 07:19 UTC (3 days, one continuous session) |
| **Project** | `e:\CS202\Super-Mario-Game` — C++17 + SFML 3.0.2, MinGW-w64 / CMake |
| **Repository state** | Branch `integration/merge-tmp-feat`, later a detached `HEAD` at merge commit `03a9923` |
| **Substantive prompts** | 14 (excluding model switches, interrupts and system notifications) |

### Model switching

The model was changed twice mid-session using the `/model` command:

| Time (UTC) | Change | Context |
|---|---|---|
| 2026-08-15 21:04 | `opus-5` → `sonnet-5` | During the blocks/scenery implementation |
| 2026-08-15 21:10 | `sonnet-5` → `opus-5` | Resumed for the remainder of the work |
| 2026-08-16 07:16 | `opus-5` → `sonnet-5` | For this report |

---

## Summary Table

| # | Time (UTC) | Model | Topic | Outcome |
|---|---|---|---|---|
| 1 | 08-13 06:24 | opus-5 | Load maps from `tmp`; flagpole → next map | Feature already existed; one header line needed |
| 2 | 08-13 06:26 | opus-5 | Chain `feat1_1 → debug → debug2 → debug3` | Chain completed and verified |
| 3 | 08-13 07:19 | opus-5 | Paratroopa flight, hammer arc, 16px tile + window sizing | 59 files changed; all verified |
| 4 | 08-13 15:24 | opus-5 | Stomp bounce too strong | Regression found (self-inflicted) and removed |
| 5 | 08-13 15:34 | opus-5 | Mario not shrinking on damage | Pre-existing bug found and fixed |
| 6 | 08-15 20:06 | opus-5 | Asset loading audit / asset manager | Full audit delivered; implementation deferred |
| 7 | 08-15 20:36 | opus-5 | 4-landscape block & scenery spec | Coordinates derived and pixel-verified |
| 8 | 08-15 21:06 | sonnet-5 | Create 4 test maps | 4 maps created and chained |
| 9 | 08-15 21:20 | opus-5 | Locate boot-map setting | Single line identified |
| 10 | 08-16 06:42 | opus-5 | Recover lost files; stair block bug | Work recovered from dangling commit; bug fixed |
| 11 | 08-16 06:54 | opus-5 | Where to commit | Guidance on detached `HEAD` |
| 12 | 08-16 06:58 | opus-5 | Conceptual: what detached `HEAD` means | Explanation, misconception corrected |
| 13 | 08-16 07:03 | opus-5 | Coin as a placeable map object | New `MapCoin` entity implemented |
| 14 | 08-16 07:19 | sonnet-5 | Produce this report | This document |

---

## Detailed Log

### 1 — Map loading and flagpole progression
**Time:** 2026-08-13 06:24 UTC · **Model:** `claude-opus-5`

**Prompt.** Read the `Super-Mario-Game` project. My current branch is the integration branch.
Your task is to read from branch `tmp` in order to load some additional maps into the game.
Can you also try to make use of the existing function that moves the player to another map
when they reach the flagpole?

**Purpose.** Bring additional map files into the active branch and wire up level-to-level
progression triggered by the flagpole.

**Result.** Investigation showed nothing needed to be taken from `tmp` — its map files were
byte-identical to the current branch, and `tmp` did not even contain `feat1_1.map`. The
flagpole → next-map feature already existed end to end and was **data-driven** via a
`; next=` header in each map file, flowing through `Level` → `GameManager` →
`LevelCompleteState`. The assistant paused for a scope correction before making changes.

---

### 2 — Establishing the map chain
**Time:** 2026-08-13 06:26 UTC · **Model:** `claude-opus-5`

**Prompt.** Correction — `debug`, `debug2` and `debug3` maps already exist. Instead, please
connect and establish the progression so that `feat1_1` leads to `debug`, then `debug2`,
then `debug3`.

**Purpose.** Create a playable four-map progression chain.

**Result.** `debug → debug2 → debug3` links were already present in the map headers; only
`feat1_1.map` lacked a `; next=` entry. **One line was added.** Verification was done by
compiling a standalone harness against the real `Level`/`TileMap` classes that walked the
chain, replayed the goal-zone castle paint (which uses bounds-checked writes), and confirmed
every flagpole had floor beneath it. The state-transition path was also checked to confirm
`clear()` + `pushState()` could not leave the state stack transiently empty.

---

### 3 — Enemy behaviour and window/tile architecture
**Time:** 2026-08-13 07:19 UTC · **Model:** `claude-opus-5`

**Prompt.** Two tasks.
*(1) Enemy behaviour.* Koopa Paratroopa movement looks wrong — it should **fly**, and when
damaged it should lose its wings and become an ordinary Koopa, rather than hopping around.
Also, the Hammer Bro's projectile needs **less gravity**; currently it feels like it drops
immediately after being thrown.
*(2) Camera and window size.* Tile width and height should be fixed at **16×16**. The screen
size (width and height) should be changeable in multiples of 16 — provide **three preset
sizes plus a fullscreen option**. The number of rows displayed must always stay the same,
and you may assume width ≥ height. This means fractional scaling can be removed in favour of
simple width/height scaling.

**Purpose.** Correct two enemy behaviours and re-architect the rendering resolution so the
logical frame matches the true pixel-art resolution.

**Result.** One clarifying question was asked (whether a wider window shows *more level* or
the same view letterboxed) because the two readings implied materially different work.

- **Paratroopa** now flies: gravity disabled, horizontal cruise plus a bounded 2-tile
  vertical patrol anchored to spawn altitude; losing wings restores gravity.
- **Hammer** given a `0.4` gravity scale — arc changed from ~1.7 tiles / 0.53 s to
  4.2 tiles / 1.30 s.
- **Tile size 32 → 16**, requiring every world-space length, velocity, acceleration and
  epsilon to be halved (88 scripted replacements, each required to match exactly once).
- **Window system rewritten**: fixed 16-row logical height, integer scaling derived from
  height, column count from window aspect, fullscreen letterboxed.

**59 files changed.** Correctness was demonstrated by re-running a physics simulation before
and after the rescale: every measurement expressed **in tiles was identical**, while all
pixel values halved. All four display modes were exercised at runtime and confirmed via log
output.

---

### 4 — Stomp bounce regression
**Time:** 2026-08-13 15:24 UTC · **Model:** `claude-opus-5`

**Prompt.** Did you change something about the character when he stomps on an enemy? Why does
he bounce? I think the player should keep moving normally and not bounce off.

**Purpose.** Investigate an unexpected change in stomp behaviour and remove the bounce.

**Result.** The assistant confirmed this was **its own regression**: the bounce was a
hard-coded `-350.0f` written inline in `CollisionManager`, which the rescale table had missed
while halving the gravity around it — raising the bounce from 1.2 to 4.8 tiles.

Removing it required two supporting changes, since the bounce was the only thing separating
the player from the enemy: a **one-stomp-per-contact lockout** (otherwise a Koopa ran its
entire shell ladder in a fraction of a second) and **holding that lockout while overlapped**
(otherwise the player landed inside the shell and was damaged by a stomp he had won).
Verified with a simulation, including a guard confirming un-stomped enemies still deal
contact damage.

---

### 5 — Player form not reverting on damage
**Time:** 2026-08-13 15:34 UTC · **Model:** `claude-opus-5`

**Prompt.** The game forgets to update the Mario sprite. When I am in the regular (large)
Mario form and take damage, the form does not change back to small Mario. Can you check?

**Purpose.** Diagnose and fix the power-down visual/state bug.

**Result.** Confirmed **pre-existing**, not caused by earlier work (the file diff was empty).
Two code paths change form, but only one resized the body: powering up went through
`setState()` which calls `syncPowerSize()`, whereas `takeDamage()` swapped the state object
without resizing. The bug was worse than cosmetic — the player retained a **two-tile
collision box** and could still **break bricks** while nominally small. Fixed with one line.
The test was proven non-vacuous by temporarily disabling the fix and observing failure.

---

### 6 — Asset loading audit
**Time:** 2026-08-15 20:06 UTC · **Model:** `claude-opus-5`

**Prompt.** How are we loading assets, and can we create an asset manager?

**Purpose.** Understand the current asset pipeline and evaluate centralising it.

**Result.** A `view::AssetManager` already existed but managed only the UI font. The audit
found **22 independent texture loads from 6 unique files** per level construction
(`enemies-8.png` decoded 10×), roughly 16.3 MiB of GPU textures for 4.5 MiB of unique data,
three `sf::Font` copies defeating the singleton, and silent failure handling. A key
constraint was identified: colour keying **must remain per-rectangle**, because the
underwater hill's body is painted the same colour that serves as the kelp's transparency key.
Planning was interrupted by the next task; **implementation remains outstanding.**

---

### 7 — Blocks and scenery for four landscapes
**Time:** 2026-08-15 20:36 UTC · **Model:** `claude-opus-5`

**Prompt.** Introducing a nearly complete set of blocks and scenery. There are **four
landscapes**: overworld, underground, castle and underwater. A map declares its landscape and
background colour, which selects the sprite images — a brick block behaves identically in
every world but looks slightly different. All of these load from `super_mario_asset`, so the
transparent colour should match the others from that sheet.

*Overworld:* two brick types already exist; add a **stair block** at (0, 33), 16×16, behaving
exactly like a non-breakable brick. Scenery: a **bush** of three consecutive 16×16 images
starting at (0, 213) with 1px gaps, occupying one map cell which is the middle image; a
**hill** of nine blocks in a pyramid — bottom layer five images from (0, 247), middle layer
reusing the 1st/3rd/5th bottom tiles, top layer one block at (34, 230), centred on its cell.

*Underground:* normal block (181, 16), brick (147, 16), stair (147, 33); scenery as overworld.
*Underwater:* brick starts at (147, 100), other blocks keep the same relative offsets;
scenery likewise, bush starting at (164, 297); plus **kelp** at (215, 297) 16×16 appearing in
stacks of 3–5, with transparent colour (66, 66, 255) — different from the other images.
Please print the positions you believe the remaining underwater images occupy.
*Castle:* same concept — brick (0, 100), bush (0, 297).

Background colours: overworld unchanged; underground and castle black; underwater
(55, 114, 255). Ask me questions if needed.

**Purpose.** Add a new `Underground` landscape and a full per-world block/scenery tile set.

**Result.** Rather than assuming, the assistant **verified every coordinate against the
actual PNG pixels** using image analysis, confirming the pattern `brick = (bx, by)`,
`solid = +34x`, `stair = +17y` across all four worlds, and discovering the scenery forms an
exact **2×2 grid** (origins `x ∈ {0,164}`, `y ∈ {213,297}`) whose four quadrants have
pixel-identical silhouettes differing only in palette. Two findings were reported back:
underground has its *own* scenery variant (contradicting the "same as overworld" assumption),
and the underwater hill's body is painted the same `(66,66,255)` given as the kelp's key —
meaning a whole-sheet colour key would erase the hill. Four clarifying questions resolved the
hill anchor, cloud/tree handling, kelp stacking and the ground tile. Implementation added the
`Underground` world type, new tile symbols, multi-cell centred tiles, a two-pass scenery
render, and batched colour-key uploads.

---

### 8 — Test maps for each landscape
**Time:** 2026-08-15 21:06 UTC · **Model:** `claude-sonnet-5`

**Prompt.** I will check the visuals myself. Create four small maps, one for each landscape,
so I can test them.

**Purpose.** Produce hand-testable maps exercising every new tile type per world.

**Result.** Four maps created (`test_overworld`, `test_underground`, `test_underwater`,
`test_castle`), chained via `; next=` headers and exercising stairs, hills, bushes, kelp,
pipes, blocks and enemies.

---

### 9 — Locating the boot map
**Time:** 2026-08-15 21:20 UTC · **Model:** `claude-opus-5`

**Prompt.** Where is the code that changes which map is entered first?

**Purpose.** Find the entry point controlling the starting level.

**Result.** Identified `GameManager::DefaultMapPath` (`GameManager.cpp:5`), noting it is used
both for the initial value and by `reset()` on returning to the menu.

---

### 10 — Recovering lost work and the stair block bug
**Time:** 2026-08-16 06:42 UTC · **Model:** `claude-opus-5`

**Prompt.** During the process I switched between branches and have now lost the newly updated
files relating to the maps. Can you re-add those? Also, the stair block does not work well —
I am not sure whether it is physics or collision, but the player seems to fall off when
standing on a stair block.

**Purpose.** Restore lost work and diagnose a gameplay bug.

**Result.** Rather than retyping the work, the assistant searched the reflog and found the
files intact in commit `1e37195` — a **dangling commit** created while on a detached `HEAD`,
referenced by no branch and eligible for garbage collection. All 17 files were restored and a
branch `recovered/world-assets` was created to protect it.

The stair bug was a genuine defect: the codebase keeps **two separate predicates** — one for
what blocks movement (`TileMap::isSolidTile`) and one for what supports the feet
(`CollisionManager::isGroundTile`). The stair had been added to the first but not the second,
so it stopped the player sideways and overhead yet never held him up. Fixed, with a comment
on both lists. Verified by simulation, and proven non-vacuous by temporary reversion
(before: feet fell to y=511, airborne 60/60 frames; after: identical to plain ground).

---

### 11 — Where to commit
**Time:** 2026-08-16 06:54 UTC · **Model:** `claude-opus-5`

**Prompt.** I do not follow the explanation about the commit — so where do I commit now?

**Purpose.** Get concrete version-control guidance.

**Result.** Explained that the current position was exactly `origin/dev`'s tip but with no
branch attached, and that the local `dev` branch was 54 commits stale. Recommended
`git switch -c <name>` — which attaches a branch without touching any files, so the
uncommitted work was never at risk.

---

### 12 — Understanding detached HEAD
**Time:** 2026-08-16 06:58 UTC · **Model:** `claude-opus-5`

**Prompt.** So does detached `HEAD` mean you are only inspecting, rather than editing and
writing?

**Purpose.** Clarify the underlying Git concept to avoid repeating the mistake.

**Result.** Corrected the misconception: detached `HEAD` is **not read-only** — editing and
committing work normally; the only difference is that no branch name follows the commits.
Clarified why the outcome looked backwards: uncommitted *edits* are just files and survive a
branch switch, whereas *commits* become orphaned. Rule given: **do not commit while
detached.**

---

### 13 — Coins as placeable map objects
**Time:** 2026-08-16 07:03 UTC · **Model:** `claude-opus-5`

**Prompt.** Next, I want to add a feature changing how we perceive coins. A coin should not
only be obtainable by hitting a coin block, but should also be an object that can occupy one
cell on the map. A player passing through it should collect it. Is this different from the way
we are currently building coins?

**Purpose.** Determine whether the existing `Coin` class could serve as a collectible pickup,
and implement placeable coins.

**Result.** Answered directly: **yes, entirely different.** The existing `Coin` is not a
collectible at all — `CoinBlock` credits the coin and score at the moment of the bump and
spawns `Coin` purely as a flourish, with a deliberately empty `onCollect()` and a trigger
hitbox that the collision pass skips. It differs from a pickup on six counts.

A new `MapCoin : Item` was added, reusing the existing pickup path
(`Item::onCollision → onCollect`). Symbol `$` chosen deliberately over lowercase `o` to avoid
confusion with `O` (cloud) in hand-written maps. Verified by simulation: static, collected
exactly once despite multi-frame overlap, +1 coin / +200 score, passable. The user's
instruction to treat three sprite frames as an animation was followed, though pixel analysis
showing identical silhouettes (a colour pulse rather than a spin) was reported back, with the
frame table kept to a single editable line.

---

### 14 — This report
**Time:** 2026-08-16 07:19 UTC · **Model:** `claude-sonnet-5`

**Prompt.** I need to write a report on AI usage covering the conversation between us. Can you
write it in markdown? Include basic information such as the model, and for each message the
prompt time, model, a complete and detailed summary of my prompt, a summary of the prompt's
purpose, and a summary of the result.

**Purpose.** Produce an auditable record of AI usage for project documentation.

**Result.** This document, compiled from the session transcript on disk so that all
timestamps and per-message model attributions are factual rather than reconstructed.

---

## Observations on Working Practice

**Verification approach.** Changes were generally not accepted on inspection alone. Standalone
harnesses were compiled against the real project classes to measure actual behaviour — enemy
trajectories, stomp resolution, stair landing, coin collection and map-chain integrity.
Where a bug was fixed, the test was frequently proven non-vacuous by temporarily reverting
the fix and confirming failure.

**Errors introduced by AI.** One regression was introduced and later corrected: an inline
`-350.0f` stomp bounce missed during the unit rescale (see entry 4). It was found by the user
during play, and the assistant identified it as its own error rather than attributing it
elsewhere. A related near-miss — the player's hitbox literal — was caught by a follow-up sweep
before reaching the user.

**Pre-existing bugs surfaced.** Independently of the requested work, the assistant identified:
the power-down resize bug (entry 5); `isStompable()` overridden by three enemies but never
called anywhere; `Goomba::onStomped` awarding no score; and dead members in
`LevelCompleteState`. Only those in scope were changed; the rest were reported for the team to
decide on.

**Assumption checking.** On at least two occasions the assistant's own session memory was
stale after the repository moved on, and re-reading the working tree corrected it before that
error propagated into the work.

---

## Outstanding Items

| Item | Status |
|---|---|
| Asset manager implementation | Audited and designed; **not implemented** |
| `isStompable()` never checked | Reported; **not changed** (alters gameplay) |
| `Goomba::onStomped` awards no score | Reported; **not changed** |
| Four test maps | Created but **not play-tested** for difficulty/feel |
| Per-landscape tile visuals | Rendered correctly in isolation; **not visually confirmed in-game** |
| Current work | **Uncommitted**, on a detached `HEAD` |
