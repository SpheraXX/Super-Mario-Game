#ifndef MODEL_GAMEMANAGER_H
#define MODEL_GAMEMANAGER_H

namespace model {

// Singleton holding the global, cross-state game progress (score, lives, level).
// Accessed everywhere through GameManager::instance().
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

    int getCurrentLevel() const;
    void setCurrentLevel(int level);
    void nextLevel();

    // Restore starting values for a brand new game.
    void reset();

    static constexpr int StartingLives = 3;
    static constexpr int FirstLevel = 1;

private:
    GameManager() = default;

    int score = 0;
    int lives = StartingLives;
    int currentLevel = FirstLevel;
};

}

#endif
