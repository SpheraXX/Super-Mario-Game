classDiagram

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
        #const TileMap$1$ mapPtr
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
        +getStateName() string
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

    class PlayerState {
        <<interface>>
        +update(player, dt)*
        +onEnter(player)*
        +onExit(player)*
        +takeDamage(player) PlayerState$1$*
        +getAnimState(player) PlayerAnimState
        +getStateName() string
        +getRemainingTime() float
        +checkExpiration() unique_ptr~PlayerState~
    }

    class SmallState {
        +takeDamage(player) PlayerState$1$*
        +getStateName() string
    }

    class SuperState {
        +takeDamage(player) PlayerState$1$*
        +getStateName() string
    }

    class FireState {
        -float fireCooldown
        +canShoot() bool
        +shoot()
        +takeDamage(player) PlayerState$1$*
        +getStateName() string
    }

    class StarState {
        -float duration
        -unique_ptr~PlayerState~ previousState
        +StarState(unique_ptr~PlayerState~)
        +takeDamage(player) PlayerState$1$*
        +checkExpiration() unique_ptr~PlayerState~
        +getRemainingTime() float
        +getStateName() string
    }

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
        +Entity$1$ other
    }

    class TileMapRenderer {
        -Texture tilesetTexture
        -unordered_map~char, IntRect~ tileRects
        +TileMapRenderer(texturePath)
        +registerTile(symbol, col, row)
        +render(window, map)
    }

    class AppEngine {
        -RenderWindow window
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
        #StateManager$1$ manager
        +onEnter()
        +onExit()
        +handleEvent(event)*
        +update(dt)*
        +render(window)*
        +isTransparent() bool
    }

    class MenuState {
        -Font font
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    class PlayState {
        -TileMap map
        -unique_ptr~TileMapRenderer~ renderer
        -Font font
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    class GameOverState {
        -Font font
        +onEnter()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }

    Entity <|-- Character
    Entity <|-- Block
    Character <|-- Player
    Character <|-- NPC
    Player <|-- Mario
    Player <|-- Luigi
    NPC <|-- Princess
    NPC <|-- MushroomRetainer
    Block <|-- CoinBlock

    PlayerState <|-- SmallState
    PlayerState <|-- SuperState
    PlayerState <|-- FireState
    PlayerState <|-- StarState

    GameState <|-- MenuState
    GameState <|-- PlayState
    GameState <|-- GameOverState

    AppEngine *-- StateManager : owns
    AppEngine *-- RenderWindow : owns
    StateManager o-- GameState : stack of
    PlayState *-- TileMap : owns
    PlayState o-- TileMapRenderer : owns
    Player o-- PlayerState : state
    StarState o-- PlayerState : wraps
    Character --> TileMap : references

    PlayState ..> GameManager : reads-slash-writes
    PlayerState ..> Player : operates on
    TileMapRenderer ..> TileMap : reads
    CollisionResult --> Entity : points to