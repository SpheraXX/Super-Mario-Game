# Map Format Report — Symbol Reference for Authoring Levels

**Purpose.** This document is the complete key binding table for `.map` files, written so a
level can be transcribed from an original Super Mario Bros. screenshot without reading any
C++. Everything here was verified against the loader
([TileMap.cpp](../src/Model/Map/TileMap.cpp)), the entity spawner
([LevelScene.cpp:258](../src/Controller/LevelScene.cpp:258)), the enemy factory
([EnemyFactory.cpp](../src/Model/Enemy/EnemyFactory.cpp)) and the tile renderer
([TileMapRenderer.cpp](../src/View/Map/TileMapRenderer.cpp)) — not against comments.

---

## 1. The single most important rule

> **The map file is written upside-down. The FIRST grid line in the file is the BOTTOM row
> of the screen.**

`TileMap::tileOrigin` computes `y = (15 - row) * 16`, and row 0 is the first grid line read
from the file. So you author a level **bottom-up**: ground strip first, then the walk line,
then the block rows, and the sky/clouds on the last lines of the file.

If you transcribe a screenshot top-to-bottom you will get an inverted level. Transcribe the
screenshot **from its bottom edge upward**.

| | |
|---|---|
| Grid height | exactly **16 rows** per area (`TileMap::Rows`), non-negotiable |
| Tile size | **16 x 16** px (`TileWidth` / `TileHeight`) |
| Screen height | 16 x 16 = **256 px** — a level is exactly one screen tall, never scrolls vertically |
| Screen width | variable (display-mode dependent); the camera scrolls horizontally |
| Grid width | free — taken from the **first grid row** of the area |
| Row 0 | first line in file = **bottom** of screen |
| Row 15 | last line in file = **top** of screen |
| Column 0 | leftmost, `x = column * 16` |

Every row must be **at least as long as row 0**; a shorter row throws
`"Area grid row is shorter than the first row"`. Characters past the width of row 0 are
ignored.

---

## 2. File anatomy

A `.map` file is plain text made of two kinds of lines:

* **Header lines** — begin with `;`. Metadata and structure. (`;` is used instead of `#`
  because `#` is already the brick tile.)
* **Grid lines** — everything else. Exactly 16 per area, in the order described above.

Blank lines are skipped. There are two loaders:

* `Level::loadFromFile` — **what the game actually uses**. Multi-area, header lines may
  appear before *or* after an area's grid.
* `TileMap::loadFromFile` — single-area legacy path. Headers must all come **before** the
  16 grid rows.

### Skeleton

```
; name=WORLD 1-1                      <- level metadata (before the first area)
; next=assets/maps/test_underground.map

; area                                <- opens area 0 (optional if only one area)
; world=overworld
<16 grid rows, bottom row first>
; pipe=col:28,enter:down,to:1:8       <- tokens bind to the area they sit with

; area                                <- opens area 1
; world=underground
<16 grid rows>
; pipe=col:8,enter:down,to:0:28
```

### Header keys

| Key | Scope | Meaning |
|---|---|---|
| `; name=` / `; level=` | level | Display name shown by the HUD |
| `; next=` | level | Path to the next level's `.map`, loaded on completion |
| `; area` | — | Bare marker: starts a new area segment |
| `; world=` | area | `overworld` \| `underground` \| `underwater` \| `castle` — picks the palette **and** the physics theme |
| `; pipe=` | area | Warp portal binding (§6) |
| `; slider=` | area | Moving-platform motion binding (§7) |

**Constraint:** in a multi-area level, the **last area must be `overworld`**, or loading
throws. Single-area files may be any world.

The default map the game boots is `assets/maps/test_castle.map`
(`GameManager::DefaultMapPath`, [GameManager.cpp:5](../src/Model/Core/GameManager.cpp:5)).

---

## 3. Key binding table — the full symbol set

### 3.1 Solid terrain (resolved by the tile pass — blocks **everything**, player and enemies)

| Key | Name | Notes |
|---|---|---|
| `G` | Ground / solid block | The workhorse. Two rows of it form the standard ground strip. A gap = write `.` |
| `s` | Stair block | Unbreakable brick. Used for the staircases before a flagpole |
| `P` | Pipe mouth, **left** cell | |
| `Q` | Pipe mouth, **right** cell | |
| `p` | Pipe shaft, **left** cell | Repeat downward for height |
| `q` | Pipe shaft, **right** cell | |

These are the *only* solid tiles (`TileMap::isSolidTile`). Anything not on this list does
not stop movement — including the castle, which is scenery (§3.4, §5).

### 3.2 Entities spawned from a tile (each manages its own collision)

| Key | Spawns | Behaviour |
|---|---|---|
| `M` | Mario spawn point | The **first** `M` found wins; later ones are ignored. If a map has no `M`, a fallback spawn is placed at column 2 on the ground |
| `C` | `?` / Coin Block | Bump from below once: 75% coin, 15% mushroom, 5% fire flower, 5% star. Then renders as a used block |
| `#` or `B` | Brick Block | Identical — both spawn a `BrickBlock`. Bumped by small Mario it bounces; by super Mario it shatters into shards |
| `$` | Free-standing coin | `MapCoin`. Collected by walking through it — never solid, never drawn as terrain. (`$` not `o`, so it can't be confused with `O` the cloud) |
| `E` | Level goal | End-of-level trigger. **Author-placed, and there is no automatic placement** — a map with no `E` cannot be completed. The hitbox is **4 tiles tall** growing upward from the marked cell, so a jumping player can't skip it. Trigger only; it never blocks |
| `=` | Slider (moving platform) | Written as a **2-cell horizontal run** (`==`). Its art is a fixed 32x8. Motion comes from a matching `; slider=` token — see §7. **Without a matching token it silently does not spawn** |

### 3.3 Enemies — the digits `0`–`9`

Enemy placement is **entirely** map-driven; `EnemyFactory` is the only place an enemy is
constructed. Digits are stripped to empty tiles at load, so **a spawn marker is never solid
ground** — you can safely put one in mid-air over a gap and it will fall.

| Key | Enemy | Status / placement |
|---|---|---|
| `0` | Goomba | |
| `1` | Koopa Troopa | |
| `2` | Koopa Paratroopa | The winged, hopping Koopa |
| `3` | Hammer Bro | |
| `4` | Lakitu | |
| `5` | Spiny | |
| `6` | Cheep Cheep | **Not implemented** — logs a warning and skips the spawn |
| `7` | Bowser | **2 tiles tall**, foot-aligned (see below) |
| `8` | Piranha Plant | Special anchor: the marker goes in the **empty cell directly above the pipe's top-left cell**, and the plant is placed one cell lower. It cannot go *on* the mouth — the loader would strip that cell to air and punch a hole in the pipe. Still flagged as deferred in the factory header; use with care |
| `9` | — | Unassigned: logs `unknown enemy id` and skips |

**Foot alignment.** A marker means *"this enemy's feet rest on the bottom edge of this
cell."* Anything one tile tall is unaffected; a taller body (Bowser) is dropped by its
overhang so it doesn't float. So: **put the digit in the empty cell directly above the
ground**, not on the ground itself.

### 3.4 Scenery — decorative, never solid, drawn **behind** terrain

| Key | Name | Size | Anchor |
|---|---|---|---|
| `O` | Cloud | 3 x 2 cells | top-left cell |
| `T` | Small tree | 1 x 2 cells | top-left cell |
| `w` | Bush | 3 x 1 cells | **centre** — the marker is the middle cell |
| `m` | Hill | 5-3-1 pyramid | **middle of the bottom row** — place it on the ground line and it grows upward |
| `k` | Kelp | 1 x 1 | own cell. **Underwater areas only** — in any other world it is unregistered and draws nothing. A tall strand is several `k` cells stacked |
| `v` | Lava surface (wave crest) | 1 x 1 | own cell |
| `x` | Lava body | 1 x 1 | own cell. Stack under a `v` — a 3-deep pool is one `v` over two `x` |
| `A` | Castle **upper** (tower) | 3 x 2 cells | top-left cell. See §5 |
| `H` | Castle **lower** (base) | 5 x 3 cells | top-left cell. See §5 |

`w` and `m` are the **only** two symbols in the whole format that paint outside their own
cell in a centred way. Everything else is anchored at its own cell (extending right/down
for the multi-cell `O`, `T`, `A` and `H`).

> ⚠️ **Lava is purely decorative.** `v`/`x` are not in `isSolidTile` and carry no damage
> logic — Mario walks straight through a lava pool. It is a backdrop, not a hazard.

**Automatic, no symbol:** in an `underwater` area the renderer paints a surface-wave tile
across row 14 (second row from the top of the screen) in every column the author left as
`.`. Reserve that row with your own terrain if you don't want it.

### 3.5 Empty

| Key | Meaning |
|---|---|
| `.` | Empty air — the canonical blank |
| `-` | Accepted as "empty" only by the pipe-width check; otherwise behaves like any unknown char |
| *anything else* | Draws nothing, blocks nothing. Silently ignored |

Because unknown characters are silently ignored, **a typo is invisible** — it produces a
hole in your level rather than an error.

---

## 4. Pipes in detail

A pipe is **terrain**, not an entity, and that is deliberate: the entity collision pass only
ever resolves the *player* against solid entities, so a pipe modelled as an entity would be
invisible to enemies and they'd walk through it. As tiles, every character collides.

Screen layout (top to bottom):

```
PQ      <- mouth
pq      <- shaft
pq      <- shaft, repeat for height
```

**In the file (bottom-up) you therefore write the `pq` lines FIRST and the `PQ` line LAST.**
See [debug3.map](../assets/maps/debug3.map) for a worked example.

Rules:

1. Write **both** columns. Run detection scans only the left column (`P`/`p`); the right
   column must be `Q`/`q` for a full 2-tile-wide pipe. (An empty cell or `-` is also
   accepted there, for backwards compatibility with older single-column maps, in which case
   the pipe is still treated as 2 wide.) A non-empty foreign character on the right makes
   the pipe **1 tile wide**.
2. A pipe is scenery unless a `; pipe=` token names its column. Only then is a `Pipe` entity
   spawned, which is what makes "hold Down to enter" work.

### `; pipe=` token

```
; pipe=col:<column>,enter:down|up,to:<areaIndex>:<destColumn>
```

| Field | Meaning |
|---|---|
| `col` | The **left** column of the pipe run in *this* area |
| `enter` | `down` (classic: stand on top, press Down) or `up`. Informational |
| `to` | Destination **area index (0-based)** and the column the player re-emerges at |
| `dest` | Alias for `to` |

Portals are one-way as written — author the return token in the destination area if you
want a two-way warp.

---

## 5. The castle — two keys

The castle is drawn from **two multi-cell images**, not a grid of one-cell tiles.

| Key | Part | Size | Atlas rect |
|---|---|---|---|
| `A` | Upper — the tower | 3 x 2 cells (48 x 32 px) | (40, 696) .. (87, 727) |
| `H` | Lower — the base | 5 x 3 cells (80 x 48 px) | (24, 728) .. (103, 775) |

Both are anchored at their **own top-left cell**, exactly like the cloud and the tree.

**Placement.** The tower is inset one cell from the base's left edge, so a complete castle
is the `A` marker **one column right** of the `H` marker and **two screen-rows above** it.
Because the file is bottom-up, that means: write the `H` line *first*, skip one line, then
write the `A` line one column further right.

Screen view, and the same thing as it appears in the file:

```
   on screen                    in the file (bottom-up)

   .A.....       row 6          .................. row 7
   .......       row 5          ............A..... row 6   <- A at column 12
   H......       row 4          .................. row 5
   .......       row 3          ...........H...... row 4   <- H at column 11
   .......       row 2          .................E row 2
   GGGGGGG    <- ground         GGGGGGGGGGGGGGGGGG row 0-1
```

See [flagpolecastle.map](../assets/maps/flagpolecastle.map) for the worked fixture.

**The castle is decorative — it is not solid and it does not end the level.** `E`
(`LevelGoal`) is what ends a level; the castle is backdrop, and Mario walks in front of it
the same way he walks past a hill. This is also why it can be two keys at all: a 2-symbol
castle has no way to express per-cell collision, and making only the marker cell solid
would read as an invisible wall.

> **History.** This used to cost **21 symbols** — `A D F H I J L N r R S U V W X Y Z a b c d`,
> most of the alphabet — painted row-major over a 5x5 silhouette, back when the castle was
> the level's ending. Old maps written in that scheme will not render correctly; re-author
> them with `A` and `H`. The freed letters are now available for future map symbols.

---

## 6. Sliders (moving platforms)

Grid: a 2-cell run of `=`. The run's **leftmost column** binds it to its motion token.

```
; slider=col:<column>,axis:h|v,dist:<worldUnits>,speed:<unitsPerSecond>
```

| Field | Meaning |
|---|---|
| `col` | Leftmost column of the `=` run |
| `axis` | `h` / `horizontal`, or `v` / `vertical` |
| `dist` | Total travel in world units (px) from one end to the other |
| `speed` | World units per second |

Example, from [test_castle.map](../assets/maps/test_castle.map):

```
; slider=col:48,axis:h,dist:64,speed:30
; slider=col:57,axis:v,dist:96,speed:25
```

A `=` run with no matching token spawns **nothing at all** — no error, no platform.

---

## 7. What a map must have — authoring checklist

**Structural (loading fails without these)**

- [ ] Exactly 16 grid lines per area
- [ ] Every row at least as wide as row 0
- [ ] At least one area
- [ ] Last area of a multi-area level is `overworld`

**Playable (loads fine, but the level is broken without these)**

- [ ] `M` — a spawn point (otherwise a fallback drops Mario at column 2)
- [ ] `E` — a goal, or the level can never be completed
- [ ] Ground: two `G` rows at the bottom is the house standard; pits are `.` in both rows
- [ ] `; world=` — otherwise you silently get `overworld` palette and physics
- [ ] `; name=` for the HUD, `; next=` to chain to the following level
- [ ] Every `; pipe=` / `; slider=` token's `col` actually matches a pipe/`=` run in that area

**Content layers, in the order it's easiest to transcribe from a screenshot**

1. Ground strip and pits (rows 0–1)
2. Terrain: `s` staircases, `PQ`/`pq` pipes, `G` platforms
3. Blocks: `C` `?`-blocks, `#`/`B` bricks, `$` loose coins
4. Enemies: digits, in the empty cell **above** their footing
5. Scenery: `w` `m` `T` `O` (and `k` if underwater, `v`/`x` for lava)
6. Goal `E` — and, if the level closes on one, the castle backdrop (`H` then `A`, §5)

**Sizing reference.** The existing maps run 18 tiles ([flagpolecastle.map](../assets/maps/flagpolecastle.map),
a fixture) to 203 tiles ([feat1_1.map](../assets/maps/feat1_1.map)). Original SMB 1-1 is
about 212 tiles long, so a faithful transcription is ~200+ characters per line.

---

## 8. Gotchas — the list of things that will bite you

1. **The file is upside-down.** Repeated because it is the number one source of wrong maps.
2. **Enemy digits are not terrain.** They're erased to air at load. This is a feature (a
   marker can never accidentally become a floor), but it means you cannot use a digit cell
   for anything else.
3. **A typo is silent.** Unknown symbols draw nothing and block nothing. Nothing warns you.
4. **`=` without a `; slider=` token spawns nothing.** Silently.
5. **A pipe without a `; pipe=` token is scenery.** Correct for decorative pipes, a bug if
   you meant it to warp.
6. **Lava does not hurt and does not hold you up.** Purely decorative (§3.4).
7. **Kelp `k` only renders underwater.** In any other world it's an invisible no-op.
8. **The castle is backdrop, not an ending.** `A`/`H` block nothing and trigger nothing —
   a castle with no `E` next to it is a level you cannot finish.
9. **`C` and `#` are not registered in the tile renderer** — they draw as entities. Don't go
   looking for their rects in `TileMapRenderer`.
10. **Legacy dead symbol `K`.** The debug maps once used letters for enemies (`E` Goomba,
    `K` Koopa) before the digit format won. `E` has since been reassigned to the level goal.
    A stray `K` still sits at column 38 of the walk line in
    [test_overworld.map](../assets/maps/test_overworld.map),
    [test_underwater.map](../assets/maps/test_underwater.map) and
    [test_castle.map](../assets/maps/test_castle.map) — it renders as nothing and should be
    `1` (Koopa). Do not copy it into new maps.

---

## 9. Quick reference card

```
TERRAIN (solid)          ENTITIES                 ENEMIES (digits)
  G  ground                M  Mario spawn           0  Goomba
  s  stair block           C  ? block               1  Koopa Troopa
  P Q  pipe mouth L/R      # B  brick               2  Koopa Paratroopa
  p q  pipe shaft L/R      $  coin                  3  Hammer Bro
                           E  level goal            4  Lakitu
                           == slider (2 cells)      5  Spiny
SCENERY (decorative)                                6  Cheep Cheep (NOT IMPL)
  O  cloud        3x2 TL                            7  Bowser (2 tiles tall)
  T  tree         1x2 TL                            8  Piranha Plant (above pipe)
  A  castle tower 3x2 TL                            9  unused
  H  castle base  5x3 TL   (A = H's column +1, 2 rows above)
  w  bush         3x1 CENTRE
  m  hill         5-3-1 pyramid, marker = bottom centre
  k  kelp         underwater only     EMPTY:  .  air
  v  lava crest
  x  lava body           HEADERS
                           ; name=   ; next=   ; area
                           ; world=overworld|underground|underwater|castle
                           ; pipe=col:N,enter:down,to:AREA:COL
                           ; slider=col:N,axis:h|v,dist:N,speed:N
```
