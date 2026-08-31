# Enemy Behaviour Reference

Behaviour spec for the enemy roster, based on the original *Super Mario Bros.* (NES).
Where we deliberately depart from the original, it is called out as **[deviation]**.

Two project-wide rules frame everything below:

1. **No enemy placement lives in C++.** Every enemy in a level comes from a digit in the map
   file. `EnemyFactory` turns those digits into objects; nothing else constructs an enemy.
2. **Runtime construction is limited to consequences.** A thrown hammer, a Spiny Egg, and the
   Spiny that egg becomes cannot be placed by a level author, so emitters create them through
   `model::World::spawn`. This is the only sanctioned exception to rule 1.

## Map enumeration

| Id | Enemy | Status |
|----|-------|--------|
| 0 | Goomba | implemented |
| 1 | Koopa Troopa | implemented |
| 2 | Koopa Paratroopa | implemented |
| 3 | Hammer Bro | implemented |
| 4 | Lakitu | implemented |
| 5 | Spiny | implemented |
| 6 | Cheep Cheep | deferred — water levels come later |
| 7 | Bowser | implemented |
| 8 | Piranha Plant | deferred — pending pipe-attachment design |

Projectiles (Hammer, Spiny Egg, Fireball) are **not** in this table. They have no map symbol
because a level author cannot meaningfully place a hammer in mid-flight.

We do not model red/green variants. **[deviation]** In the original, colour selects between
patrol styles (red Koopas turn at ledges, green ones walk off) and Paratroopa flight paths.
One behaviour per enemy is enough for this clone, and it halves the art and state.

---

## 0. Goomba

Walks in one direction at constant speed, reverses on walls, walks off ledges. Stomping
squashes it into a flat sprite that lingers briefly, then despawns.

## 1. Koopa Troopa

Walks like a Goomba. Stomping retracts it into a shell rather than killing it — the Koopa
object persists through all shell states, which is why the shell is not its own class.

- **Shell idle**: stationary, harmless until touched.
- **Shell spinning**: kicked away at high speed in the direction opposite the player. Knocks
  out any enemy it touches, ricochets off walls, and is stopped by stomping it again.
- **[deviation]** Shells never revive. The original SMB1 shell is also permanent; the wobble
  and re-emergence people remember is from SMB3 and Super Mario World.

## 2. Koopa Paratroopa

A winged Koopa. Stomping removes the wings and leaves an ordinary walking Koopa — a second
stomp then produces the shell. Two stomps to shell, three to kick.

In the original, green Paratroopas hop in the player's general direction; only World 7-3 has
genuinely flying ones. We implement the hop: a normal walk plus a fixed-height bounce
whenever it lands.

## 3. Hammer Bro

Patrols a small fixed area, hopping very frequently — it hops even while idle — and throws a
continuous supply of hammers. In the original it is usually placed in pairs on stacked brick
platforms it can hop between.

- Patrol is a short beat around its spawn point, not a full-map walk.
- It faces and throws toward the player, so hammers are aimed rather than fired blind.
- Hammers travel in an arc: launched up and forward, then falling under gravity.
- **[deviation]** Hammers ignore terrain entirely (`usesTileCollision() == false`), matching
  the original's behaviour of arcing over and through the brick structures.
- Stompable, and among the hardest enemies in the game because the hammer supply is endless.

## 4. Lakitu

Rides a cloud above the player, drifting back and forth to stay overhead while dropping an
endless supply of Spiny Eggs. No gravity, no tile collision — it floats over all terrain.

It is the only enemy that steers by the player rather than by the world, which is what
`World::getPlayer()` exists for.

**[deviation] — the eggs.** In the shipped NES ROM, Spiny Eggs drop straight down with no
horizontal motion. Disassembly showed this to be a *bug*: the intended code computed a
trajectory from Mario's position and speed, Lakitu's speed, and a random factor, throwing the
egg ahead of the player to cut off their progress, and bouncing it off blocks on the way down.
We implement the intended arc, because a straight drop from a cloud that is already directly
overhead is trivially avoidable.

Lakitu can be stomped.

## 5. Spiny

Walks like a Goomba once it lands. Its shell is covered in spikes, so **it cannot be stomped**
— landing on one damages the player instead of squashing it. Killed by a spinning shell or,
later, a fireball.

Spinies normally reach the level only by hatching from a Lakitu's egg. We still give them a
map id so levels can place them directly, which the original never does. **[deviation]**

### Spiny Egg (projectile, no map id)

Thrown by Lakitu, falls under gravity, and damages the player on contact in the air. On
touching the ground it is replaced by a Spiny at the same position. This transformation is
why `World::spawn` takes an `Entity` and not a `Projectile`.

## 7. Bowser

Paces along a short stretch, jumps periodically, and breathes fire. Later castles add thrown
hammers to the pattern.

- Not stompable — landing on him damages the player.
- Fire travels horizontally at a constant height, ignoring terrain and gravity.
- In the original he is defeated either by the axe at the end of the bridge or by five
  fireballs. We keep the five-hit health pool; **[deviation]** the axe and the collapsing
  bridge are level scripting and are out of scope for the enemy system.

## 6. Cheep Cheep — deferred

Swims in water levels with near-zero gravity; a leaping variant jumps out of the water. Needs
the water/swimming mechanic first, so it is not implemented yet. `EnemyFactory` returns
`nullptr` for id 6 and logs a warning.

## 8. Piranha Plant — deferred

Rises from and retracts into a pipe on a fixed timer, never stompable, damaging on all sides.
Requires `usesTileCollision() == false` so the pipe it lives inside does not eject it, plus a
pipe-mouth anchor captured at spawn and a render order that draws it behind the pipe.

Held back pending the pipe-attachment design. **[deviation]** When it lands, it will emerge on
a pure timer regardless of whether the player is standing on the pipe; the original suppresses
emergence when Mario is close, which is what stops a pipe from becoming an unavoidable hit.

---

## Tuning table

Speeds are world units per second (one tile = 32 units). These are feel-first starting values,
not measurements from the ROM.

| Enemy | Walk | Gravity scale | Tile collision | Stompable | Emits | Cooldown |
|-------|------|---------------|----------------|-----------|-------|----------|
| Goomba | 40 | 1.0 | yes | yes | — | — |
| Koopa | 40 (shell 250) | 1.0 | yes | yes | — | — |
| Koopa Paratroopa | 40 + hop | 1.0 | yes | yes | — | — |
| Hammer Bro | 30 patrol | 1.0 | yes | yes | Hammer | 2.0 s |
| Lakitu | 60 tracking | 0.0 | no | yes | Spiny Egg | 3.0 s |
| Spiny | 50 | 1.0 | yes | **no** | — | — |
| Bowser | 30 | 1.0 | yes | **no** | Fireball | 2.5 s |

| Projectile | Gravity scale | Tile collision | On landing |
|------------|---------------|----------------|------------|
| Hammer | 1.0 | no | — (falls out of the world) |
| Spiny Egg | 1.0 | yes | becomes a Spiny |
| Fireball | 0.0 | no | — (leaves the camera) |

## Sources

- [Hammer Bro — Super Mario Wiki](https://www.mariowiki.com/Hammer_Bro)
- [Koopa Paratroopa — Super Mario Wiki](https://www.mariowiki.com/Koopa_Paratroopa)
- [Spiny — Super Mario Wiki](https://www.mariowiki.com/Spiny)
- [Spiny Egg — Super Mario Wiki](https://www.mariowiki.com/Spiny_Egg)
- [Super Mario Bros./Enemies — StrategyWiki](https://strategywiki.org/wiki/Super_Mario_Bros./Enemies)
- [How Lakitu throws Spiny Eggs is due to a glitch — Nintendo Everything](https://nintendoeverything.com/how-lakitu-throws-spiny-eggs-in-super-mario-bros-is-due-to-a-glitch-not-the-intended-behavior/)
- [Spiny Egg Behavior Fix for SMB1 — Romhacking.net](https://www.romhacking.net/hacks/3238/)
