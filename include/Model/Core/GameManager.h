#ifndef MODEL_GAMEMANAGER_H
#define MODEL_GAMEMANAGER_H

#include <string>

namespace model {

// Singleton holding the global, cross-state game progress (score, lives, level, coins,
// the current map and the one that follows it). Accessed everywhere through
// GameManager::instance().
class GameManager {
public:
    static GameManager& instance();

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    int getScore() const;
    void addScore(int points);

    int getLives() const;
    void loseLife();
    void addLife();
    bool isGameOver() const;

    int getCoins() const;
    void addCoin();

    // The map currently being played. PlayState loads it on enter and, when the level
    // is cleared, advances to the map declared by the finished one ('; next=...').
    const std::string& getCurrentMapPath() const;
    void setCurrentMapPath(const std::string& path);
    const std::string& getNextMapPath() const;
    void setNextMapPath(const std::string& path);
    const std::string& getLevelName() const;
    void setLevelName(const std::string& name);

    // Score earned at the end of the finished level (flag + time bonus), shown by the
    // LevelComplete overlay.
    int getLevelClearBonus() const;
    void setLevelClearBonus(int bonus);

    int getCurrentLevel() const;
    void setCurrentLevel(int level);
    void nextLevel();

    // Restore starting values for a brand new game.
    void reset();

    static constexpr int StartingLives = 3;
    static constexpr int FirstLevel = 1;
    static constexpr int CoinsPerLife = 100;
    // The map the game boots into (AppEngine starts directly in PlayState).
    static const char* DefaultMapPath;

private:
    GameManager() = default;

    int score = 0;
    int lives = StartingLives;
    int coins = 0;
    int currentLevel = FirstLevel;
    int levelClearBonus = 0;
    std::string currentMapPath = DefaultMapPath;
    std::string nextMapPath;
    std::string levelName;
};

}

#endif
