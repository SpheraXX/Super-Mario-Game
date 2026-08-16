# M2a — Class Reference cho Player Characters

> Tài liệu này dành cho người đảm nhận **M2b (Enemies, Items, Blocks)**.
> Hiểu cấu trúc class M2a để tích hợp code của bạn cho đúng.

---

## 1. Sơ đồ kế thừa

```
Entity                     (model/Entity.h)
├── Character              (model/Character.h)
│   ├── Player             (model/Player.h)
│   │   ├── Mario          (model/Mario.h)
│   │   └── Luigi          (model/Luigi.h)
│   └── NPC                (model/NPC.h)
│       ├── Princess       (model/Princess.h)
│       └── MushroomRetainer (model/MushroomRetainer.h)
└── Block                  (model/Block.h)
    └── CoinBlock          (model/CoinBlock.h)
```

**Lưu ý:** M2b cần tạo thêm `Enemy` (kế thừa `Character`) và các class con của nó.

---

## 2. Entity — Gốc của mọi thứ

**File:** `include/Model/Entity.h`, `src/Model/Entity.cpp`

```cpp
namespace model {
struct Vector2 { float x; float y; };

class Entity {
public:
    Entity(Vector2 position, Vector2 size);
    virtual ~Entity() = default;
    virtual void update(float deltaTime);

    Vector2 getPosition() const;
    Vector2 getSize() const;
    void setPosition(Vector2 newPosition);

private:
    Vector2 position;   // topLeft corner
    Vector2 size;       // width × height
};
}
```

**M2b cần biết:**
- Mọi thực thể trong game (enemy, item, block) **phải kế thừa Entity**.
- `position` là **góc trên-trái** (không phải center).
- `update(deltaTime)` là `virtual` — override để thêm logic riêng.
- Dùng `getPosition()` / `setPosition()` để thay đổi vị trí.

---

## 3. Character — Động vật có chuyển động

**File:** `include/Model/Character.h`, `src/Model/Character.cpp`

```cpp
class Character : public Entity {
public:
    Character(Vector2 position, Vector2 size);

    void update(float deltaTime) override;   // gravity + position update
    virtual void render(sf::RenderWindow& window);
    virtual void onCollision(Entity* other);
    virtual void takeDamage(int amount);

    void applyGravity(float deltaTime);
    bool isOnGround() const;
    void setMap(const TileMap* map);         // GÁN MAP TRƯỚC KHI DÙNG

    Vector2 getVelocity() const;
    void setVelocity(Vector2 v);
    int getDirection() const;                // -1 = trái, +1 = phải
    void setDirection(int d);
    bool isAlive() const;
    AnimState getAnimState() const;
    bool isFacingRight() const;

    void resolveTileCollisions();            // Va chạm với tile map

protected:
    Vector2 velocity;
    int direction;
    int health;
    bool alive;
    AnimState animState;       // Idle, Walk, Run, Jump, Fall, Die
    bool facingRight;
    const TileMap* mapPtr;     // Pointer tới map (không sở hữu)

    static constexpr float Gravity = 980.0f;       // pixel/s²
    static constexpr float MaxFallSpeed = 600.0f;
};
```

**M2b cần biết cho Enemy:**

| Thành phần | Ý nghĩa | M2b cần làm gì |
|---|---|---|
| `velocity` | Vận tốc hiện tại (x = ngang, y = dọc) | Set velocity để enemy di chuyển |
| `health` | Máu, mặc định = 1 | Enemy có thể override để nhiều máu hơn |
| `alive` | Còn sống không | Gọi `takeDamage()` để giảm health, tự set `alive=false` khi hết máu |
| `animState` | Trạng thái animation | Set Idle/Walk/Jump/Fall/Die |
| `applyGravity()` | Áp dụng trọng lực | Gọi trong `update()` override |
| `isOnGround()` | Kiểm tra đang đứng trên đất | Dùng để biết khi nào nhảy/dừng |
| `setMap()` | Gán tile map để va chạm | **BẮT BUỘC** gọi khi tạo enemy |
| `resolveTileCollisions()` | Xử lý va chạm với ô đất | Gọi sau `update()` mỗi frame |
| `onCollision(Entity* other)` | Gọi khi va chạm thực thể khác | Override để xử lý enemy va chạm player/item |
| `takeDamage(int amount)` | Trừ health, tự chết khi health ≤ 0 | Player đạp enemy → gọi `enemy->takeDamage(1)` |

**`update()` mặc định của Character:**
```cpp
void Character::update(float deltaTime) {
    if (!alive) return;          // Chết rồi thì không update
    applyGravity(deltaTime);     // velocity.y += 980 * dt
    pos.x += velocity.x * dt;   // Di chuyển ngang
    pos.y += velocity.y * dt;   # Di chuyển dọc
    setPosition(pos);
}
```

**Luồng xử lý 1 frame cho Enemy (M2b cần làm):**
```
enemy->handleInput()          // (nếu có) — enemy tự di chuyển
enemy->update(deltaTime)      // gravity + position
enemy->resolveTileCollisions() // va chạm map
enemy->render(window)         // vẽ
```

---

## 4. Player — Nhân vật điều khiển được

**File:** `include/Model/Player.h`, `src/Model/Player.cpp`

```cpp
class Player : public Character {
public:
    Player(Vector2 position, Vector2 size);

    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void handleInput();                      // Đọc bàn phím
    void onCollision(Entity* other) override;
    void takeDamage(int amount) override;

    // State pattern
    void setState(std::unique_ptr<PlayerState> newState);
    PlayerState& getState();
    const char* getStateName() const;
    float getRemainingTime() const;

    // Power-up
    void becomeSuper();
    void becomeFire();
    void becomeStar();

    // Stats
    void addScore(int points);
    void addCoin();       // 100 coins = 1 life
    void addLife();
    int getScore() const;
    int getCoins() const;
    int getLives() const;

protected:
    std::unique_ptr<PlayerState> state;   // State pattern
    int score;
    int coins;
    int lives;               // Mặc định = 3
    float damageCooldown;    // 0.5s无敌 sau khi nhận damage
};
```

**M2b cần biết:**
- `onCollision(Entity* other)` — đây là nơi Player xử lý va chạm với enemy/item/block.
  Hiện tại hàm này **đang rỗng**. Khi M2b thêm enemy, bạn cần **override** hoặc **gọi từ trong hàm này** để xử lý:
  - Player nhảy lên đầu enemy → enemy chết, player được nhảy lên
  - Player chạm enemy từ bên → player nhận damage
  - Player chạm item → collect item

- `takeDamage(int amount)` — Player override Character::takeDamage. Logic:
  1. Hỏi `state->takeDamage()` → state quyết định chuyển xuống state nhỏ hơn hay không
  2. Nếu trả về `nullptr` → mất 1 life, nếu lives > 0 thì respawn, nếu = 0 thì chết
  3. Nếu trả về state mới → chuyển state, bật damageCooldown 0.5s

- `damageCooldown` — sau khi nhận damage, player bất tử trong 0.5s. Kiểm tra:
  ```cpp
  if (!alive || damageCooldown > 0.0f) return;  // trong takeDamage()
  ```

---

## 5. Mario & Luigi — Hai nhân vật playable

**Mario** (`include/Model/Mario.h`):
```cpp
class Mario : public Player {
public:
    Mario(Vector2 position);    // size = {16, 16}
    static constexpr float WalkSpeed = 180.0f;
    static constexpr float RunSpeed  = 320.0f;
    static constexpr float JumpForce = -450.0f;   // âm = nhảy lên
};
```

**Luigi** (`include/Model/Luigi.h`):
```cpp
class Luigi : public Player {
public:
    Luigi(Vector2 position);    // size = {16, 16}
    static constexpr float WalkSpeed = 160.0f;     // Chậm hơn Mario
    static constexpr float RunSpeed  = 280.0f;
    static constexpr float JumpForce = -540.0f;    // Nhảy cao hơn Mario
};
```

**So sánh:**

| | Mario | Luigi |
|---|---|---|
| WalkSpeed | 180 | 160 |
| RunSpeed | 320 | 280 |
| JumpForce | -450 | -540 (nhảy cao hơn) |
| Màu render | Red | Green |

**M2b cần biết:** Khi kiểm tra va chạm enemy ↔ player, bạn không cần phân biệt Mario/Luigi vì cả hai đều là `Player`. Dùng `dynamic_cast<Player*>(entity)` để kiểm tra.

---

## 6. PlayerState — State Pattern

**File:** `include/Model/PlayerState.h`, `src/Model/PlayerState.cpp`

Player có 4 trạng thái, mỗi trạng thái định nghĩa hành vi khác nhau:

```
PlayerState (interface)
├── SmallState     — trạng thái mặc định (nhỏ)
├── SuperState     — sau khi ăn mushroom (lớn)
├── FireState      — sau khi ăn fire flower (lớn + bắn lửa)
└── StarState      — sau khi ăn star (bất tử 10s, bọc state trước)
```

**StarState đặc biệt:** nó "bọc" (wrap) state trước đó. Khi hết 10s → tự động quay lại state cũ.

```cpp
class PlayerState {
public:
    virtual void update(Player& player, float deltaTime) = 0;
    virtual void onEnter(Player& player) = 0;
    virtual void onExit(Player& player) = 0;
    virtual PlayerState* takeDamage(Player& player) = 0;
    virtual const char* getStateName() const = 0;
    virtual float getRemainingTime() const;          // -1 = vô hạn
    virtual std::unique_ptr<PlayerState> checkExpiration();  // nullptr = chưa hết hạn
};
```

**Bảng xử lý damage:**

| State hiện tại | `takeDamage()` trả về | Kết quả |
|---|---|---|
| SmallState | `nullptr` | Mất 1 life → respawn hoặc game over |
| SuperState | `new SmallState()` | Thu nhỏ lại |
| FireState | `new SmallState()` | Thu nhỏ lại |
| StarState | `this` (chính nó) | **Bất tử** — không bị gì |

**M2b cần biết:** Khi enemy tấn công player, gọi `player->takeDamage(1)` — state pattern sẽ tự xử lý. Bạn không cần quan tâm player đang ở state nào.

---

## 7. NPC — Đối tượng có thể tương tác

**File:** `include/Model/NPC.h`, `src/Model/NPC.cpp`

```cpp
class NPC : public Character {
public:
    NPC(Vector2 position, Vector2 size, const std::string& dialogue);
    virtual void interact(Player& player);    // Override trong con

    std::string getDialogue() const;
    bool isInteractable() const;
    void setInteractable(bool value);

protected:
    std::string dialogue;
    bool interactable;
};
```

**Hai NPC hiện có:**

| Class | Dialogue | Score khi interact |
|---|---|---|
| `Princess` | "Thank you Mario!" | +5000 |
| `MushroomRetainer` | "The Princess is in another castle!" | +1000 |

Cả hai tự tắt `interactable = false` sau khi tương tác.

**M2b cần biết:**
- NPC kế thừa Character nên có gravity, velocity, position — nhưng NPC hiện **không tự di chuyển** (không override update).
- Nếu M2b cần enemy có thể tương tác, có thể kế thừa NPC hoặc tạo class mới từ Character.

---

## 8. Block & CoinBlock — Các ô trên bản đồ

**Block** (`include/Model/Block.h`):
```cpp
class Block : public Entity {
public:
    Block(Vector2 position, Vector2 size, char tileSymbol);
    char getTileSymbol() const;
    bool isSolid() const;
private:
    char tileSymbol;
    bool solid;
};
```

**CoinBlock** (`include/Model/CoinBlock.h`):
```cpp
class CoinBlock : public Block {
public:
    CoinBlock(Vector2 position, Vector2 size);
    bool hasCoin() const;
    void collectCoin();
private:
    bool coinAvailable;
};
```

**M2b cần biết:**
- Block kế thừa **Entity** (không phải Character) — không có gravity, velocity.
- M2b sẽ cần tạo thêm `QuestionBlock`, `Brick`, `Pipe` — kế thừa `Block`.
- CoinBlock đã có sẵn logic `collectCoin()` — dùng khi player nhảy đập block.

---

## 9. CollisionResult — Kết quả va chạm

**File:** `include/Model/CollisionResult.h`

```cpp
enum class CollisionType { None, Top, Bottom, Left, Right };

struct CollisionResult {
    bool collided = false;
    CollisionType type = CollisionType::None;
    Entity* other = nullptr;
};
```

**M2b cần biết:** Dùng struct này khi kiểm tra va chạm enemy ↔ player để biết va chạm từ hướng nào. Ví dụ:
- `type == Top` → player nhảy lên đầu enemy → enemy chết
- `type == Left/Right` → player chạm bên → player nhận damage

---

## 10. Hướng tích hợp M2b vào M2a

### Tạo Enemy kế thừa Character

```cpp
// include/Model/Enemy.h
class Enemy : public Character {
public:
    Enemy(Vector2 position, Vector2 size);
    void update(float deltaTime) override;
    void render(sf::RenderWindow& window) override;
    void onCollision(Entity* other) override;

    virtual void onStomped();     // Player nhảy lên đầu
    virtual void onHit();         // Player tấn công từ bên

protected:
    int damage;
    bool active;
};

// include/Model/Goomba.h
class Goomba : public Enemy {
public:
    Goomba(Vector2 position);
    void onStomped() override;    // Chết
};
```

### Xử lý va chạm Player ↔ Enemy

Trong `Player::onCollision(Entity* other)` hoặc trong game loop:

```cpp
void Player::onCollision(Entity* other) {
    if (auto* enemy = dynamic_cast<Enemy*>(other)) {
        if (velocity.y > 0 && getPosition().y + getSize().y < enemy->getPosition().y + 8) {
            // Player đang rơi + above enemy → nhảy lên đầu
            enemy->onStomped();
            velocity.y = JumpForce * 0.6f;   // Bật lên
            addScore(100);
        } else if (damageCooldown <= 0.0f) {
            // Player chạm bên → nhận damage
            takeDamage(1);
        }
    }

    if (auto* block = dynamic_cast<CoinBlock*>(other)) {
        if (velocity.y < 0 && /* head hit block */) {
            if (block->hasCoin()) {
                block->collectCoin();
                addCoin();
            }
        }
    }
}
```

### Tạo Entity trong game loop

```cpp
// Trong PlayState hoặc GameManager
auto goomba = std::make_unique<model::Goomba>(model::Vector2{300.0f, 400.0f});
goomba->setMap(&map);    // BẮT BUỘC
enemies.push_back(std::move(goomba));

// Mỗi frame:
for (auto& enemy : enemies) {
    enemy->update(deltaTime);
    enemy->resolveTileCollisions();
    // Kiểm tra va chạm với player...
}
```

---

## 11. Các hàm quan trọng cần gọi đúng thứ tự

```
Tạo thực thể:
  entity = new XxxEntity(position, size);
  entity->setMap(&map);              // CHỈ cho Character subclass

Mỗi frame:
  entity->update(deltaTime);         // gravity + position
  entity->resolveTileCollisions();   // va chạm map (CHỈ cho Character)
  entity->render(window);            // vẽ

Khi va chạm:
  entity->onCollision(other);        // xử lý logic va chạm
  player->takeDamage(1);             // player nhận damage
  enemy->takeDamage(1);              // enemy nhận damage
```

---

## 12. Directory structure

```
include/
  Model/
    Entity.h              ← Gốc
    Character.h           ← Có gravity, velocity, map collision
    Player.h              ← Có state, score, lives, input
    Mario.h               ← WalkSpeed/RunSpeed/JumpForce
    Luigi.h               ← WalkSpeed/RunSpeed/JumpForce khác
    PlayerState.h         ← SmallState, SuperState, FireState, StarState
    NPC.h                 ← NPC có dialogue + interact
    Princess.h            ← NPC con
    MushroomRetainer.h    ← NPC con
    Block.h               ← Block cơ bản
    CoinBlock.h           ← Block có coin
    CollisionResult.h     ← Struct kết quả va chạm
    TileMap.h             ← Bản đồ tile
    GameManager.h         ← Singleton quản lý level/score/lives
  View/
    TileMapRenderer.h     ← Vẽ bản đồ
  Controller/
    AppEngine.h           ← Game loop chính
    StateManager.h        ← Quản lý state stack
    GameState.h           ← Interface cho game state
    PlayState.h           ← State đang chơi
    MenuState.h           ← State menu
    GameOverState.h       ← State game over

src/
  Model/                  ← Implementation tương ứng
  View/
  Controller/
```
