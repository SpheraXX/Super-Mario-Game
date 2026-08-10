# Class Encapsulation & Inter-class Connections

> One-liner per class on what it encapsulates, followed by the methods that connect it to other classes.
> Golden rule: encapsulation in a single phrase. Companion doc: `FULL_CLASS_DIAGRAM.md`.

## Model Layer

### Core value types
- **Vector2** — plain x/y pair with no behaviour. No connections.
- **Hitbox** — owns its box geometry and the AABB queries against another box. → `intersects(Hitbox, myPos, otherPos)`, `getOverlap(Hitbox, myPos, otherPos)`.
- **CollisionResult** — result bag for a collision. → holds `Entity* other`.
- **BlockHitEvent** — event bag for a bump. → holds `Entity& player`, `CollisionType side`, `float upwardSpeed`.

### Entity hierarchy
- **Entity** — base of everything on a level: hides position/size, exposes the behaviour-hook contract. → `onCollision(Entity&, CollisionType)`, `onTileCollision(char, CollisionType)`, `onTriggerEnter(Entity&)`, `onBlockHit(BlockHitEvent&)`, `onStomped(Entity&)`, `onHit(Entity&)`, `takeDamage(int)`.
- **Character** — movement/state of living things (direction, health, death, animation pose, world physics). → `setMap(TileMap*)`, `resolveTileCollisions()` (reads grid), `setWorld(World&)` + `getGravity()/getMaxFallSpeed()/getHorizontalDrag()/isUnderwater()` (reads World), `onCollision(Entity*)`.
- **Player** — wraps the power-up state machine and jump/input intent; score wrappers forward to the global state. → `setState(std::unique_ptr<PlayerState>)`, `getState()`; `addScore()/addCoin()/addLife()/getScore()/getCoins()/getLives()` → GameManager; `getInputDown()` read by PlayState (pipe entry).
- **Mario / Luigi** — tuning-only: speed/jump constants and their getters (consumed polymorphically by Character/Player). No cross-class methods.
- **Enemy** — damage value, stomp/squish lifecycle and despawn timer. → `onStomped(Entity&)`, `onHit(Entity&)` (player); `updateAI(dt)` is self-contained.
- **Goomba** — walk AI + squish + turning at walls. → `onTileCollision(char, CollisionType)` (reads tile symbols), `onStomped(Entity&)`.
- **Koopa** — shell state machine (walk/idle/spin). → `onStomped(Entity&)`, `onCollision(Entity&, CollisionType)` (shell hits other entities), `onTileCollision(char, CollisionType)`.
- **NPC** — dialogue text + interactability flag. → `interact(Player&)`.
- **Princess / MushroomRetainer** — character-flavoured NPCs. → override `interact(Player&)`.
- **Block** — tile symbol, solidity, and the render-side bounce timer. → `onBlockHit(BlockHitEvent&)` (dispatched by CollisionManager), `getBounceOffsetY()` read by renderers.
- **CoinBlock** — coin availability + pop-out animation. → `onBlockHit(BlockHitEvent&)`; `isOpened()/isCoinPopping()/getCoinPopProgress()` read by CoinBlockRenderer.
- **BrickBlock** — always-bouncing solid brick. → `onBlockHit(BlockHitEvent&)`.

### PlayerState (State pattern)
- **PlayerState** — power-up behaviour interface (update/damage/anim). → `update(Player&, dt)`, `takeDamage(Player&)`, `getAnimState(Player&)`, `checkExpiration()`.
- **SmallState / SuperState** — plain power-up behaviours. → `takeDamage(Player&)` returns the next state.
- **FireState** — shooting cooldown. → `shoot()`/`canShoot()` via Player, `takeDamage(Player&)`.
- **StarState** — timed invincibility wrapping the previous state. → owns `std::unique_ptr<PlayerState> previousState`; `checkExpiration()` returns it back.

### Level / map / world
- **TileMap** — owns the 16-row grid + metadata and all grid queries. → `loadFromLines()` filled by Level; `getTile()/isSolidTile()/getColumns()/getRows()` read by CollisionManager, PlayState, renderers; `padRight()` called by PlayState for the completion zone.
- **WorldType** — enum + string parse. → `worldTypeFromString(std::string)` produces a WorldType.
- **World** — immutable world descriptor (colors, tileset, physics scales). → getters read by Character (physics) and TileMapRenderer (theme).
- **WorldSet** — world registry/factory. → `forType(WorldType)` returns the World descriptor.
- **Level** — owns the parsed multi-area list (grid + world + portals per area). → `areaMap()/areaWorld()/portals()/areaCount()` read by PlayState; `loadFromFile()`.
- **Portal** — value struct linking a source pipe column to a destination area/column. → `sourceColumn` matched by PlayState against `Pipe::getSourceColumn()`.
- **Pipe** — solid warp pipe carrying its portal anchor column. → `isSolid()` (CollisionManager), `getSourceColumn()` (PlayState entry/one-way logic).
- **FlagPole** — trigger hitbox + touched flag + slide progress for the clear cinematic. → `onTriggerEnter(Entity&)` (CollisionManager trigger pass), `get/setSlideProgress()` (PlayState ↔ FlagPoleRenderer).
- **Castle** — solid, non-landable level cap. → `isSolid()/isLandable()` (CollisionManager).

### Game-flow services
- **GameManager** — singleton global progress (score, lives, coins, map chain, clear bonus). → `instance()` consumed by PlayState, Player wrappers, GameOverState, LevelCompleteState.
- **LevelTimer** — level countdown state. → `update()/pause()/resume()/reset()` driven by PlayState; `getRemainingSeconds()` read for the HUD snapshot.
- **CollisionManager** — owns collision resolution against grid and entity pairs. → `update(std::vector<Entity*>&, dt)` (drives Entity hooks), `resolveEntityInteraction(Entity&, Entity&, side)` (dispatches BlockHitEvent / onTriggerEnter / push-out), `calculateSide()`.

## View Layer

- **RenderContext** — read-only per-frame bag (world type) for renderers. → carries `WorldType`.
- **EntityRenderer** — render strategy interface. → `render(RenderTarget&, Entity&, ctx)`.
- **TypedEntityRenderer\<T\>** — safe downcast to the concrete entity type. → `renderTyped(RenderTarget&, T&, ctx)`.
- **SpriteEntityRenderer\<T\>** — owns a texture + shared character-frame drawing. → `drawCharacterFrame()` reads `Character` API (position, size, `isFacingRight()`).
- **EntityRendererRegistry** — owns renderers keyed by entity type. → `registerRenderer<T,R>()` (constructs R), `render(entity)` (dispatches by `typeid`).
- **EntityRenderUtils** — free sprite helpers. → `setupEntitySprite(sf::Sprite, frame, size, mirror)`; scale constants read TileMap sizes.
- **PlayerRenderer / GoombaRenderer / KoopaRenderer** — sprite renderers of characters. → read Player/Goomba/Koopa anim state + facing.
- **PipeRenderer / FlagPoleRenderer / CastleRenderer / CoinBlockRenderer / BrickBlockRenderer** — typed entity renderers. → read their entity's position/size and state (CoinBlock opened/pop, Block bounce offset, FlagPole slide).
- **TileMapRenderer** — owns tilesets and the symbol→rect map. → `render(RenderTarget&, TileMap&)` reads the grid; `registerTile()`; theme from WorldType.
- **HudData** — plain snapshot struct. → filled by PlayState, read by HudRenderer.
- **HudRenderer** — draws the top bar from the snapshot. → `render(RenderTarget&, HudData&)`.
- **HitboxRenderer** — debug overlay. → `render(Entity&)`, `renderTiles(TileMap&)`.
- **AssetManager** — singleton owner of shared assets (UI font). → `getUiFont()` consumed by Menu/GameOver/LevelComplete states.
- **TextUtils** — text positioning/auto-size helpers. → operate on `sf::Font`/`sf::Text`.

## Controller Layer

- **GameState** — abstract screen contract with a back-pointer to the manager. → `manager` → StateManager (friend).
- **StateManager** — owns the state stack and deferred transitions. → `pushState()/popState()/replaceState()/clear()` + `applyPending()` drive `GameState` lifecycle; `handleEvent()/update()/render()` forward to the active state.
- **AppEngine** — owns the window, offscreen scene and state stack; runs the fixed-step loop. → `run()/update()/render()` drive StateManager; screen constants read TileMap sizes.
- **MenuState** — opening screen. → `handleEvent()` pushes PlayState via manager; uses AssetManager font.
- **PlayState** — orchestrates one playthrough: loads areas, spawns entities, runs collisions/timer/clear sequence, snapshots the HUD. → `loadArea()/teleportToPortal()` read Level/Portal and match Pipe columns (one-way via `inertPipeColumns`); `resetLevel()` spawns all model entities; `update()` drives CollisionManager, LevelTimer, Player; `render()` drives TileMapRenderer, EntityRendererRegistry, HudRenderer, HitboxRenderer; reads/writes GameManager; resolves worlds via WorldSet; pushes GameOverState / LevelCompleteState.
- **GameOverState** — final score screen. → reads GameManager; returns to menu via manager.
- **LevelCompleteState** — transparent clear overlay. → reads GameManager (bonus, next map); advances state via manager.
