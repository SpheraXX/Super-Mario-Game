#ifndef MODEL_SAVE_SAVEDATA_H
#define MODEL_SAVE_SAVEDATA_H

#include <string>
#include <vector>
#include <cstddef>

namespace model {

struct PlayerSaveData {
    float posX = 0.0f;
    float posY = 0.0f;
    float velX = 0.0f;
    float velY = 0.0f;
    bool isBig = false;
    std::string power = "None"; // "None", "Fire", "Star"
    float starDuration = 0.0f;
    int facingDirection = 1;   // 1: Right, -1: Left
    bool isLuigi = false;
};

struct TileCoord {
    std::size_t row = 0;
    std::size_t col = 0;
};

struct BlockSaveData {
    float posX = 0.0f;
    float posY = 0.0f;
    bool opened = false;
};

struct EnemySaveData {
    std::string type; // "Goomba", "Koopa", "KoopaParatroopa", "HammerBro", "Lakitu", "Spiny", "Bowser", "PiranhaPlant"
    float posX = 0.0f;
    float posY = 0.0f;
    float velX = 0.0f;
    float velY = 0.0f;
    bool isDormant = false;
    bool facingRight = true;
    int direction = 1;
    bool isWinged = false;
    std::string state = "Walking"; // "Walking", "ShellIdle", "ShellSpinning"
};

struct ItemSaveData {
    std::string type; // "Mushroom", "FireFlower", "Starman", "Coin"
    float posX = 0.0f;
    float posY = 0.0f;
    float velX = 0.0f;
    float velY = 0.0f;
    int direction = 1;
};

struct LevelSaveData {
    int currentLevel = 1;
    std::string mapPath = "assets/maps/debug.map";
    std::string nextMapPath;
    std::string levelName;
    std::size_t currentArea = 0;
    float remainingTime = 400.0f;
    std::vector<TileCoord> removedTiles;
    std::vector<BlockSaveData> coinBlocks;
    std::vector<EnemySaveData> enemies;
    std::vector<ItemSaveData> items;
    bool hasEntitiesSnapshot = false;
};

struct GameSaveData {
    int score = 0;
    int lives = 10;
    int coins = 0;
    int version = 1;
    LevelSaveData level;
    PlayerSaveData player;
};

} // namespace model

#endif // MODEL_SAVE_SAVEDATA_H
