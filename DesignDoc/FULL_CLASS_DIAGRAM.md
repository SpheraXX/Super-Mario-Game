# Full Project Class Diagram

> Mermaid diagram covering all model, view, and controller classes.

```mermaid
classDiagram
    %% ====================================================================
    %% MODEL LAYER — Entity hierarchy
    %% ====================================================================

    class Vector2 {
        +float x
        +float y
    }

    class Entity {
        #Vector2 position
        #Vector2 size
        +Entity(position, size)
        +~Entity()
        +update(dt)
        +getPosition() Vector2
        +getSize() Vector2
        +setPosition(pos)
    }

    class Character {
        #Vector2 velocity
        #int direction
        #int health
        #bool alive
        #AnimState animState
        #bool facingRight
        #const TileMap* mapPtr
        +Character(position, size)
        +update(dt)*
        +render(window)*
        +onCollision(other)
        +takeDamage(amount)
        +applyGravity(dt)
        +isOnGround() bool
        +setMap(map)
        +resolveTileCollisions()
        +clampVelocity()
        +getVelocity() Vector2
        +setVelocity(v)
        +getDirection() int
        +setDirection(d)
        +isAlive() bool
        +getAnimState() AnimState
        +setAnimState(s)
        +isFacingRight() bool
        +setFacingRight(b)
    }

    class Player {
        #unique_ptr~PlayerState~ state
        #int score
        #int coins
        #int lives
        #float damageCooldown
        +Player(position, size)
        +update(dt)*
        +render(window)*
        +handleInput()
        +onCollision(other)*
        +takeDamage(amount)*
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
    }

    class Mario {
        +Mario(position)
        +WalkSpeed 180
        +RunSpeed 320
        +JumpForce -450
    }

    class Luigi {
        +Luigi(position)
        +WalkSpeed 160
        +RunSpeed 280
        +JumpForce -540
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
        #char tileSymbol
        #bool solid
        +Block(position, size, tileSymbol)
        +getTileSymbol() char
        +isSolid() bool
    }

    class CoinBlock {
        #bool coinAvailable
        +CoinBlock(position, size)
        +hasCoin() bool
        +collectCoin()
    }

    %% --------------------------------------------------------------------
    %% MODEL LAYER — PlayerState hierarchy (State Pattern)
    %% --------------------------------------------------------------------

    class PlayerState {
        <<interface>>
        +update(player, dt)*
        +onEnter(player)*
        +onExit(player)*
        +takeDamage(player) $1$PlayerState$2$
        +getAnimState(player) PlayerAnimState
        +getStateName() const char*
        +getRemainingTime() float
        +checkExpiration() unique_ptr~PlayerState~
    }

    class SmallState {
        +takeDamage(player) PlayerState*
        +getStateName() const char*
    }

    class SuperState {
        +takeDamage(player) PlayerState*
        +getStateName() const char*
    }

    class FireState {
        -float fireCooldown
        +canShoot() bool
        +shoot()
        +takeDamage(player) PlayerState*
        +getStateName() const char*
    }

    class StarState {
        -float duration
        -unique_ptr~PlayerState~ previousState
        +StarState(unique_ptr~PlayerState~)
        +takeDamage(player) PlayerState*
        +checkExpiration() unique_ptr~PlayerState~
        +getRemainingTime() float
        +getStateName() const char*
    }

    %% --------------------------------------------------------------------
    %% MODEL LAYER — Other model classes
    %% --------------------------------------------------------------------

    class TileMap {
        -vector~vector~char~~ tiles
        +loadFromFile(path)
        +getTile(row, col) char
        +getRows() size_t
        +getColumns() size_t
        +Rows 16
        +Columns 32
        +TileWidth 64
        +TileHeight 48
    }

    class GameManager {
        <<singleton>>
        -int score
        -int lives
        -int currentLevel
        +instance() GameManager
        +getScore() int
        +addScore(points)
        +getLives() int
        +loseLife()
        +addLife()
        +isGameOver() bool
        +getCurrentLevel() int
        +setCurrentLevel(level)
        +nextLevel()
        +reset()
    }

    class CollisionResult {
        +bool collided
        +CollisionType type
        +Entity* other
    }

    %% --------------------------------------------------------------------
    %% VIEW LAYER
    %% --------------------------------------------------------------------

    class TileMapRenderer {
        -sf::Texture tilesetTexture
        -unordered_map~char, sf::IntRect~ tileRects
        +TileMapRenderer(texturePath)
        +registerTile(symbol, col, row)
        +render(window, map)
    }

    %% --------------------------------------------------------------------
    %% CONTROLLER LAYER — State pattern
    %% --------------------------------------------------------------------

    class AppEngine {
        -sf::RenderWindow window
        -StateManager states
        +AppEngine()
        +run()
        +processInput()
        +update(dt)
        +render()
        +TimeStep 1/60
    }

    class StateManager {
        -vector~unique_ptr~GameState~~ stack
        +pushState(state)
        +popState()
        +replaceState(state)
        +clear()
        +applyPending()
        +handleEvent(event)
        +update(dt)
        +render(window)
        +empty() bool
    }

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

    class MenuState {
        -sf::Font font
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    class PlayState {
        -TileMap map
        -unique_ptr~TileMapRenderer~ renderer
        -sf::Font font
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    class GameOverState {
        -sf::Font font
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    %% ====================================================================
    %% RELATIONSHIPS
    %% ====================================================================

    %% Inheritance (model)
    Entity <|-- Character
    Entity <|-- Block
    Character <|-- Player
    Character <|-- NPC
    Player <|-- Mario
    Player <|-- Luigi
    NPC <|-- Princess
    NPC <|-- MushroomRetainer
    Block <|-- CoinBlock

    %% Inheritance (PlayerState)
    PlayerState <|-- SmallState
    PlayerState <|-- SuperState
    PlayerState <|-- FireState
    PlayerState <|-- StarState

    %% Inheritance (controller GameState)
    GameState <|-- MenuState
    GameState <|-- PlayState
    GameState <|-- GameOverState

    %% Composition / Aggregation
    AppEngine *-- StateManager : owns
    AppEngine *-- "1" sf::RenderWindow : owns
    StateManager o-- "*" GameState : stack of
    PlayState *-- TileMap : owns
    PlayState o-- TileMapRenderer : owns
    Player o-- PlayerState : state (unique_ptr)
    StarState o-- PlayerState : wraps previous state
    Character --> TileMap : references (non-owning)

    %% Usage / Dependency
    PlayState ..> GameManager : reads/writes
    PlayerState ..> Player : operates on
    TileMapRenderer ..> TileMap : reads
    CollisionResult --> Entity : points to
```

## Enum Definitions

| Enum | Values |
|------|--------|
| `AnimState` | `Idle`, `Walk`, `Run`, `Jump`, `Fall`, `Die` |
| `PlayerAnimState` | `SmallIdle`, `SmallWalk`, `SmallRun`, `SmallJump`, `SmallFall`, `SmallDie`, `BigIdle`, `BigWalk`, `BigRun`, `BigJump`, `BigFall`, `FireIdle`, `FireWalk`, `FireRun`, `FireJump`, `FireFall`, `FireShoot`, `StarIdle`, `StarWalk`, `StarRun`, `StarJump`, `StarFall` |
| `CollisionType` | `None`, `Top`, `Bottom`, `Left`, `Right` |

## Legend

| Arrow | Meaning |
|-------|---------|
| `<\|--` | Inheritance |
| `*--` | Composition (owning) |
| `o--` | Aggregation (non-owning reference) |
| `..>` | Dependency / usage |