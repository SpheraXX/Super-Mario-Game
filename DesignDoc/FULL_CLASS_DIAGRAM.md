# Full Project Class Diagram

> Mermaid diagram covering all model, view, and controller classes (k.2.5). Stage 1
> (model interface segregation) and stage 2 are reflected: the goal castle is painted
> into the TileMap completion zone from a 21-symbol sheet (`TileMap::CastleSymbols`,
> registered by `TileMapRenderer` from atlas rows y = 696/712 (tower) and 728/744/760
> (base)); pipes spawn from `'p'` (body) / `'P'` (mouth) runs in the grid; tile
> collision treats castle symbols as ground/solid; and every static-image draw goes
> through the shared `SpritePainter` facade (tile map, pipes, flagpole — the `Castle`
> entity and `CastleRenderer` are gone). Still pending: stage 3, the PlayState split.
> Companion doc: `CLASS_ENCAPSULATION_AND_CONNECTIONS.md` (per-class encapsulation + inter-class methods).

```mermaid
classDiagram
    %% ====================================================================
    %% MODEL LAYER — Core value types
    %% ====================================================================

    class Vector2 {
        +float x = 0.0f
        +float y = 0.0f
    }

    class Hitbox {
        +Vector2 offset
        +float width
        +float height
        +bool isTrigger
        +CollisionLayer layer
        +intersects(other, myPos, otherPos) bool
        +getOverlap(other, myPos, otherPos) Vector2
    }

    class CollisionResult {
        +bool collided
        +CollisionType type
        +Entity* other
    }

    class BlockHitEvent {
        +Entity& player
        +CollisionType side
        +float upwardSpeed
    }

    %% ====================================================================
    %% MODEL LAYER — Entity hierarchy
    %% ====================================================================

    class Entity {
        -Vector2 position
        -Vector2 size
        +Hitbox hitbox
        +bool isActive
        +Entity(position, size)
        +update(dt)
        +onCollision(other, side)
        +onTileCollision(tile, side)
        +onTriggerEnter(other)
        +isSolid() bool
        +getPosition() Vector2
        +getSize() Vector2
        +setPosition(pos)
        +setSize(size)
    }

    class Character {
        #Vector2 velocity
        #int direction
        #int health
        #bool alive
        #bool isDyingFlag
        #AnimState animState
        #bool facingRight
        #const TileMap* mapPtr
        #const World* worldPtr
        +bool isGrounded
        +update(dt)*
        +handleInput(dt)*
        +takeDamage(amount)*
        +beginDying(bounce)
        +isDying() bool
        +isAlive() bool
        +getWalkSpeed()* float
        +getRunSpeed()* float
        +getMaxJumpSpeed()* float
        +getJumpAccel()* float
        +applyGravity(dt)
        +die()
        +isOnGround() bool
        +setMap(map)
        +setWorld(world)
        +getGravity() float
        +getMaxFallSpeed() float
        +getHorizontalDrag() float
        +isUnderwater() bool
        +getVelocity() Vector2
        +setVelocity(v)
        +getDirection() int
        +setDirection(d)
        +getAnimState() AnimState
        +setAnimState(s)
        +isFacingRight() bool
        +setFacingRight(b)
        $DefaultGravity 1600
        $DefaultMaxFallSpeed 900
        $DeathBounceSpeed -400
    }

    class Player {
        #unique_ptr~PlayerState~ state
        #float damageCooldown
        #float coyoteTime
        #float jumpBufferTime
        #float jumpHoldTime
        #bool jumpHeld
        #bool playerInitiatedJump
        #bool inputMoving
        #bool inputDown
        +update(dt)*
        +handleInput(dt)
        +takeDamage(amount)*
        +die(bounce)
        +setState(unique_ptr~PlayerState~)
        +getState() PlayerState&
        +getStateName() const char*
        +getRemainingTime() float
        +becomeSuper()
        +becomeFire()
        +becomeStar()
        +addScore(points)
        +addCoin()
        +addLife()
        +getScore() int
        +getCoins() int
        +getLives() int
        +getInputDown() bool
        -syncAnimation()
        $DamageCooldownTime 0.5
        $CoyoteTime 0.1
        $JumpBufferTime 0.1
        $MaxJumpHoldTime 0.16
        $JumpInitialSpeed -220
        $GroundAccel 800
        $AirAccel 600
        $Friction 1600
        $SwimAccel 900
        $SwimMaxSpeed 220
    }

    class Mario {
        +Mario(position)
        +getWalkSpeed()* float
        +getRunSpeed()* float
        +getMaxJumpSpeed()* float
        +getJumpAccel()* float
        $WalkSpeed 180
        $RunSpeed 360
        $MaxJumpSpeed 680
        $JumpAccel 3100
    }

    class Luigi {
        +Luigi(position)
        +getWalkSpeed()* float
        +getRunSpeed()* float
        +getMaxJumpSpeed()* float
        +getJumpAccel()* float
        $WalkSpeed 160
        $RunSpeed 350
        $MaxJumpSpeed 600
        $JumpAccel 3400
    }

    class Enemy {
        #int damageValue
        #bool isStomped
        #float despawnTimer
        +update(dt)*
        +updateAI(dt)*
        +onStomped(stomper)
        +onHit(source)
        +getDamageValue() int
        +isSquished() bool
    }

    class Goomba {
        -float squishTimer
        +Goomba(position)
        +updateAI(dt)*
        +onStomped(player)*
        +onTileCollision(tile, side)*
        $WalkSpeed 50
    }

    class Koopa {
        -KoopaState state
        -float shellSpeed
        +Koopa(position)
        +updateAI(dt)*
        +onStomped(player)*
        +onCollision(other, side)*
        +onTileCollision(tile, side)*
        +isShell() bool
        $WalkSpeed 40
        $SpinSpeed 250
    }

    class NPC {
        #string dialogue
        #bool interactable
        +NPC(position, size, dialogue)
        +interact(player)*
        +getDialogue() string
        +isInteractable() bool
        +setInteractable(b)
    }

    class Princess {
        +Princess(position)
        +interact(player)*
    }

    class MushroomRetainer {
        +MushroomRetainer(position)
        +interact(player)*
    }

    class Block {
        -char tileSymbol
        -bool solid
        #float bounceElapsed
        +Block(position, size, tileSymbol)
        +update(dt)*
        +getTileSymbol() char
        +isSolid() bool*
        +onBlockHit(event)
        +startBounce()
        +getBounceOffsetY() float
        $BounceDuration 0.22
        $BounceHeight 6
    }

    class CoinBlock {
        -bool coinAvailable
        -float coinPopElapsed
        +CoinBlock(position, size)
        +isOpened() bool
        +isCoinPopping() bool
        +getCoinPopProgress() float
        +update(dt)*
        +onBlockHit(event)*
        $CoinPopDuration 0.7
    }

    class BrickBlock {
        +BrickBlock(position, size)
        +onBlockHit(event)*
    }

    %% ====================================================================
    %% MODEL LAYER — PlayerState hierarchy (State Pattern)
    %% ====================================================================

    class PlayerState {
        <<interface>>
        +update(player, dt)*
        +onEnter(player)*
        +onExit(player)*
        +takeDamage(player)* PlayerState*
        +getAnimState(player)* PlayerAnimState
        +isSuper() bool
        +isFire() bool
        +isStar() bool
        +getStateName()* const char*
        +getRemainingTime() float
        +checkExpiration() unique_ptr~PlayerState~
    }

    class SmallState {
        +takeDamage(player)* PlayerState*
        +getStateName()* const char*
    }

    class SuperState {
        +takeDamage(player)* PlayerState*
        +getStateName()* const char*
    }

    class FireState {
        -float fireCooldown
        +takeDamage(player)* PlayerState*
        +canShoot() bool
        +shoot()
        $FireCooldownDuration 0.5
    }

    class StarState {
        -float duration
        -unique_ptr~PlayerState~ previousState
        +StarState(previous)
        +takeDamage(player)* PlayerState*
        +checkExpiration()* unique_ptr~PlayerState~
        +getRemainingTime()* float
        $StarDuration 10
    }

    %% ====================================================================
    %% MODEL LAYER — Level / map / world / game-flow services
    %% ====================================================================

    class TileMap {
        -vector~vector~char~~ tiles
        -size_t columns
        -string levelName
        -WorldType worldType
        -string nextMapPath
        +loadFromFile(path)
        +loadFromLines(rows)
        +padRight(extraColumns)
        +setTile(row, col, symbol)
        +getTile(row, col) char
        +getRows() size_t
        +getColumns() size_t
        +getLevelName() string
        +getWorldType() WorldType
        +getNextMapPath() string
        +hasNextMap() bool
        +isSolidTile(symbol) bool
        +isCastleSymbol(symbol) bool
        -parseHeader(line)
        $Rows 16
        $TileWidth 32
        $TileHeight 32
        $CloudSymbol O
        $SmallTreeSymbol T
        $CastleTiles 21
        $CastleSymbols A D F H I J L N Q R S U V W X Y Z a b c d
    }

    class World {
        -WorldType type
        -sf_Color backgroundColor
        -string tilesetPath
        -float gravityScale
        -float maxFallScale
        -float horizontalDrag
        +World(type, bgColor, tilesetPath, gravityScale, maxFallScale, drag)
        +getType() WorldType
        +getBackgroundColor() sf_Color
        +getTilesetPath() string
        +getGravityScale() float
        +getMaxFallScale() float
        +getHorizontalDrag() float
    }

    class WorldSet {
        <<factory>>
        +forType(type) World
    }

    class Level {
        -vector~Area~ areas
        -string levelName
        -string nextMapPath
        +loadFromFile(path)
        +areaCount() size_t
        +areaMap(index) TileMap
        +areaWorld(index) WorldType
        +portals(index) vector~Portal~
        +getLevelName() string
        +getNextMapPath() string
        +hasNextMap() bool
    }

    class Portal {
        +size_t sourceColumn
        +PortalDirection direction
        +size_t destinationArea
        +size_t destinationColumn
    }

    class Pipe {
        -size_t sourceColumn_
        +Pipe(position, size, sourceColumn)
        +isSolid() bool*
        +getSourceColumn() size_t
    }

    %% Pipes are spawned by PlayState from vertical 'p'/'P' runs in the grid ('p' body,
    %% 'P' mouth); adjacent columns of a tube (p/P side by side) form one wide pipe.

    class FlagPole {
        -bool touched
        -float slideProgress
        +FlagPole(position, size)
        +onTriggerEnter(other)*
        +isTouched() bool
        +getSlideProgress() float
        +setSlideProgress(progress)
    }

    %% The gameplay castle is painted into the grid by PlayState via TileMap::setTile
    %% from the CastleSymbols sheet (see TileMap); no Castle entity exists anymore.

    class GameManager {
        <<singleton>>
        -int score
        -int lives
        -int coins
        -int currentLevel
        -int levelClearBonus
        -string currentMapPath
        -string nextMapPath
        -string levelName
        +instance() GameManager
        +getScore() int
        +addScore(points)
        +getLives() int
        +loseLife()
        +addLife()
        +isGameOver() bool
        +getCoins() int
        +addCoin()
        +getCurrentMapPath() string
        +setCurrentMapPath(path)
        +getNextMapPath() string
        +setNextMapPath(path)
        +getLevelName() string
        +setLevelName(name)
        +getLevelClearBonus() int
        +setLevelClearBonus(bonus)
        +getCurrentLevel() int
        +setCurrentLevel(level)
        +nextLevel()
        +reset()
        $StartingLives 3
        $FirstLevel 1
        $CoinsPerLife 100
        $DefaultMapPath "assets/maps/debug.map"
    }

    class LevelTimer {
        -float remaining
        -bool paused
        +LevelTimer(startSeconds = 400)
        +update(dt)
        +pause()
        +resume()
        +reset(startSeconds)
        +isPaused() bool
        +isExpired() bool
        +getRemainingSeconds() int
        +getRemaining() float
    }

    class CollisionManager {
        -TileMap* tileMap
        +CollisionManager(tileMap)
        +update(entities, dt)
        +calculateSide(a, b) CollisionType
        +resolveEntityInteraction(a, b, side)
        -processTileCollisions(character, dt)
        -processEntityCollisions(entities)
        -pushOutOfBlock(moverCharacter, blocker, moverSide)
    }

    %% Tile grounding accepts 'G', block cells ('C'/'B'/'#') and the painted castle
    %% symbols (TileMap::isCastleSymbol).

    %% ====================================================================
    %% VIEW LAYER — renderer hierarchy
    %% ====================================================================

    class RenderContext {
        +WorldType worldType
    }

    class EntityRenderer {
        <<interface>>
        +render(window, entity, ctx)*
    }

    class TypedEntityRenderer~T~ {
        +render(window, entity, ctx) final
        #renderTyped(window, entity, ctx)*
    }

    class SpriteEntityRenderer~T~ {
        #sf_Texture texture
        -bool textureLoaded
        -bool sourceFacesRight
        +SpriteEntityRenderer(texturePath, sourceFacesRight = false)
        #drawCharacterFrame(window, entity, frame)
    }

    class EntityRendererRegistry {
        -unordered_map~type_index, unique_ptr~EntityRenderer~~ renderers
        +registerRenderer~T, R~(args...)
        +render(window, entity, ctx)
    }

    class EntityRenderUtils {
        $SpriteScaleX 2
        $SpriteScaleY 2
        +setupEntitySprite(sprite, frame, entitySize, mirror)
    }

    class PlayerRenderer {
        +PlayerRenderer()
        #renderTyped(window, player, ctx)*
    }

    class GoombaRenderer {
        +GoombaRenderer()
        #renderTyped(window, goomba, ctx)*
    }

    class KoopaRenderer {
        +KoopaRenderer()
        #renderTyped(window, koopa, ctx)*
    }

    class PipeRenderer {
        -SpritePainter painter
        +PipeRenderer()
        #renderTyped(window, pipe, ctx)*
    }

    class FlagPoleRenderer {
        -SpritePainter painter
        +FlagPoleRenderer()
        #renderTyped(window, pole, ctx)*
    }

    class CoinBlockRenderer {
        -sf_Texture texture
        -sf_Texture coinTexture
        +CoinBlockRenderer()
        #renderTyped(window, coinBlock, ctx)*
    }

    class BrickBlockRenderer {
        -sf_Texture texture
        +BrickBlockRenderer()
        #renderTyped(window, brickBlock, ctx)*
    }

    class SpritePainter {
        -sf_Image image
        -sf_Texture texture
        -bool loaded
        +SpritePainter()
        +SpritePainter(texturePath)
        +load(texturePath) bool
        +isLoaded() bool
        +applyColorKey(area, transparentColor)
        +draw(target, frame, position, scale)
        +drawCell(target, frame, origin)
        $SourceTileSize 16
        $ColorKeyTolerance 16
    }

    %% SpritePainter is the single sprite-drawing facade: it owns one tileset image +
    %% texture, re-uploads after color keying, and draws frames snapped to integer
    %% pixels. The tile map, pipes and the flagpole all paint through it, so sheet
    %% loading and snapping live in exactly one place.

    class TileMapRenderer {
        -unordered_map~string, SpritePainter~ tilesets
        -unordered_map~char, TileEntry~ tileRects
        +TileMapRenderer(tilesetPath, worldType)
        +loadTileset(tilesetPath)
        +registerTile(symbol, tilesetPath, x, y, w, h, transparentColor)
        +render(window, map)
    }

    %% TileMapRenderer's constructor additionally registers all 21 castle-sheet symbols
    %% (TileMap::CastleSymbols) against MarioAssetPath: tower at atlas y = 696/712
    %% (x = 40..72), base rows at y = 728/744/760 (x = 24..88).

    class HudData {
        +int score
        +int coins
        +string levelName
        +int time
    }

    class HudRenderer {
        -sf_Font font
        -bool fontLoaded
        +HudRenderer()
        +render(window, data)
    }

    class HitboxRenderer {
        +render(target, entity)
        +renderTiles(target, map)
    }

    class AssetManager {
        <<singleton>>
        -sf_Font uiFont
        -bool uiFontLoaded
        +instance() AssetManager
        +getUiFont() sf_Font
        +isFontLoaded() bool
    }

    class TextUtils {
        +snap(position) sf_Vector2f
        +drawCentered(window, text, centerX, centerY)
        +fitCharacterSize(font, content, maxWidth, preferredSize) uint
    }

    %% ====================================================================
    %% CONTROLLER LAYER — State pattern
    %% ====================================================================

    class GameState {
        <<interface>>
        #StateManager* manager
        +onEnter()
        +onExit()
        +handleEvent(event)*
        +update(dt)*
        +render(window)*
        +isTransparent() bool
    }

    class StateManager {
        -vector~unique_ptr~GameState~~ stack
        -vector~PendingChange~ pending
        +pushState(state)
        +popState()
        +replaceState(state)
        +clear()
        +applyPending()
        +handleEvent(event)
        +update(dt)
        +render(window)
        +empty() bool
        -activeState() GameState*
    }

    class AppEngine {
        -sf_RenderWindow window
        -StateManager states
        -sf_RenderTexture scene
        -sf_View fixedView
        -int scaleIndex
        +AppEngine()
        +run()
        -processInput()
        -update(dt)
        -render()
        -applyWindowScale()
        $ScreenWidth 640
        $ScreenHeight 512
        $DefaultWindowScale 1.5
        $TimeStep 1/60
    }

    class MenuState {
        -sf_Font font
        -bool fontLoaded
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    class PlayState {
        -Level level
        -size_t currentArea
        -TileMap map
        -unique_ptr~TileMapRenderer~ renderer
        -bool mapLoaded
        -vector~size_t~ inertPipeColumns
        -unique_ptr~EntityRendererRegistry~ entityRenderers
        -unique_ptr~HudRenderer~ hudRenderer
        -HitboxRenderer hitboxRenderer
        -bool showHitboxes
        -unique_ptr~CollisionManager~ collisionManager
        -vector~unique_ptr~Entity~~ entities
        -Player* player
        -WorldType worldType
        -LevelTimer timer
        -HudData hudData
        -FlagPole* flagPole
        -bool levelComplete
        -ClearPhase clearPhase
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
        -resetLevel()
        -loadArea(areaIndex)
        -teleportToPortal(portal)
        -beginLevelClear()
        -updateClearSequence(dt)
        -finishLevelClear()
        $LevelPaddingTiles 16
        $PoleOffsetTiles 6
        $CastleOffsetTiles 11
    }

    class GameOverState {
        -sf_Font font
        -bool fontLoaded
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    class LevelCompleteState {
        -sf_Font font
        -bool fontLoaded
        +handleEvent(event)
        +update(dt)
        +render(window)
        +isTransparent() bool
    }

    %% ====================================================================
    %% RELATIONSHIPS
    %% ====================================================================

    %% Inheritance (model)
    Entity <|-- Character
    Entity <|-- Block
    Entity <|-- Pipe
    Entity <|-- FlagPole
    Character <|-- Player
    Character <|-- Enemy
    Character <|-- NPC
    Player <|-- Mario
    Player <|-- Luigi
    Enemy <|-- Goomba
    Enemy <|-- Koopa
    NPC <|-- Princess
    NPC <|-- MushroomRetainer
    Block <|-- CoinBlock
    Block <|-- BrickBlock

    %% Inheritance (PlayerState)
    PlayerState <|-- SmallState
    PlayerState <|-- SuperState
    PlayerState <|-- FireState
    PlayerState <|-- StarState

    %% Inheritance (view renderers)
    EntityRenderer <|-- TypedEntityRenderer
    TypedEntityRenderer <|-- SpriteEntityRenderer
    SpriteEntityRenderer <|-- PlayerRenderer : T = Player
    SpriteEntityRenderer <|-- GoombaRenderer : T = Goomba
    SpriteEntityRenderer <|-- KoopaRenderer : T = Koopa
    TypedEntityRenderer <|-- PipeRenderer : T = Pipe
    TypedEntityRenderer <|-- FlagPoleRenderer : T = FlagPole
    TypedEntityRenderer <|-- CoinBlockRenderer : T = CoinBlock
    TypedEntityRenderer <|-- BrickBlockRenderer : T = BrickBlock

    %% Inheritance (controller)
    GameState <|-- MenuState
    GameState <|-- PlayState
    GameState <|-- GameOverState
    GameState <|-- LevelCompleteState

    %% Composition / Aggregation
    AppEngine *-- StateManager : owns
    AppEngine *-- sf_RenderWindow : owns
    AppEngine *-- sf_RenderTexture : owns scene
    StateManager o-- "*" GameState : stack of
    PlayState *-- Level : owns
    PlayState *-- TileMap : working grid
    PlayState ..> TileMap : paints castle (setTile, completion zone)
    PlayState *-- CollisionManager : owns
    PlayState o-- "*" Entity : spawns
    PlayState o-- TileMapRenderer : owns
    PlayState o-- EntityRendererRegistry : owns
    PlayState o-- HudRenderer : owns
    PlayState o-- HitboxRenderer : owns
    PlayState o-- LevelTimer : owns
    Player o-- PlayerState : state (unique_ptr)
    StarState o-- PlayerState : wraps previous
    Character --> TileMap : mapPtr (non-owning)
    Character --> World : worldPtr (non-owning)
    CollisionManager --> TileMap : resolves grid
    CollisionManager --> Entity : resolves pairs
    CollisionManager --> Character : tile pass (layer-guarded casts)
    CollisionManager --> Enemy : stomp/damage routing (Enemy layer)
    CollisionManager ..> Block : dispatches onBlockHit (dynamic_cast)
    Koopa ..> Enemy : shell knockout (layer-guarded cast)
    Level o-- "*" TileMap : per-area grids
    Level o-- "*" Portal : per-area portals
    EntityRendererRegistry o-- "*" EntityRenderer : owns
    EntityRendererRegistry ..> Entity : keys by typeid
    TypedEntityRenderer ..> Entity : downcasts to T
    SpriteEntityRenderer ..> Character : reads facing
    PipeRenderer o-- SpritePainter : owns one
    FlagPoleRenderer o-- SpritePainter : owns one
    TileMapRenderer o-- "*" SpritePainter : one per tileset path

    %% Usage / Dependency
    PlayState ..> GameManager : score/coins/lives/map path
    PlayState ..> WorldSet : theme + physics per area
    PlayState ..> GameOverState : pushes on death
    PlayState ..> LevelCompleteState : pushes on clear
    PlayState ..> HudData : fills snapshot
    PlayState ..> Character : casts for input / life-state checks
    Player ..> GameManager : thin score wrappers
    Pipe ..> Portal : sourceColumn matches
    PlayState ..> Pipe : one-way entry check
    CollisionManager ..> BlockHitEvent : dispatches to blocks
    CollisionManager ..> FlagPole : trigger pass
    BlockHitEvent --> Entity : player reference
    CollisionResult --> Entity : other
    MenuState ..> AssetManager : UI font
    GameOverState ..> AssetManager : UI font
    LevelCompleteState ..> AssetManager : UI font
    GameOverState ..> GameManager : final score
    LevelCompleteState ..> GameManager : bonus + next map
    HudRenderer ..> HudData : reads
    TileMapRenderer ..> TileMap : reads
    SpritePainter ..> TileMap : cell scale (TileWidth/SourceTileSize)
    HitboxRenderer ..> Entity : debug overlay
    HitboxRenderer ..> TileMap : debug overlay
    TileMapRenderer ..> World : theme colors
```

## Enums

| Enum | Values |
|------|--------|
| `CollisionType` | `None`, `Top`, `Bottom`, `Left`, `Right` |
| `CollisionLayer` | `Player`, `Enemy`, `Environment`, `Trigger` |
| `AnimState` | `Idle`, `Walk`, `Run`, `Jump`, `Fall`, `Die` |
| `PlayerAnimState` | `SmallIdle`, `SmallWalk`, `SmallRun`, `SmallJump`, `SmallFall`, `SmallDie`, `BigIdle`, `BigWalk`, `BigRun`, `BigJump`, `BigFall`, `FireIdle`, `FireWalk`, `FireRun`, `FireJump`, `FireFall`, `FireShoot`, `StarIdle`, `StarWalk`, `StarRun`, `StarJump`, `StarFall` |
| `KoopaState` | `Walking`, `ShellIdle`, `ShellSpinning` |
| `PortalDirection` | `Down`, `Up` |
| `WorldType` | `Overworld`, `Underwater`, `Castle` |

## Legend

| Arrow | Meaning |
|-------|---------|
| `<\|--` | Inheritance |
| `*--` | Composition (owning) |
| `o--` | Aggregation (non-owning reference) |
| `..>` | Dependency / usage |
