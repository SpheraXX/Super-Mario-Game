#include "Model/Core/GameManager.h"

namespace model {

const char* GameManager::DefaultMapPath = "assets/maps/lv1_4.map";

GameManager& GameManager::instance() {
    static GameManager singleton;
    return singleton;
}

int GameManager::getScore() const {
    return score;
}

void GameManager::addScore(int points) {
    score += points;
}

int GameManager::getLives() const {
    return lives;
}

void GameManager::loseLife() {
    if (lives > 0) {
        --lives;
    }
}

void GameManager::addLife() {
    ++lives;
}

bool GameManager::isGameOver() const {
    return lives <= 0;
}

int GameManager::getCoins() const {
    return coins;
}

void GameManager::addCoin(int count) {
    if (count <= 0) return;
    coins += count;
    // Classic rule: every 100 collected coins trade in for an extra life.
    while (coins >= CoinsPerLife) {
        coins -= CoinsPerLife;
        addLife();
    }
}

const std::string& GameManager::getCurrentMapPath() const {
    return currentMapPath;
}

void GameManager::setCurrentMapPath(const std::string& path) {
    currentMapPath = path;
}

const std::string& GameManager::getNextMapPath() const {
    return nextMapPath;
}

void GameManager::setNextMapPath(const std::string& path) {
    nextMapPath = path;
}

const std::string& GameManager::getLevelName() const {
    return levelName;
}

void GameManager::setLevelName(const std::string& name) {
    levelName = name;
}

int GameManager::getLevelClearBonus() const {
    return levelClearBonus;
}

void GameManager::setLevelClearBonus(int bonus) {
    levelClearBonus = bonus;
}

int GameManager::getCurrentLevel() const {
    return currentLevel;
}

void GameManager::setCurrentLevel(int level) {
    currentLevel = level;
}

void GameManager::nextLevel() {
    ++currentLevel;
}

void GameManager::reset() {
    score = 0;
    lives = StartingLives;
    coins = 0;
    currentLevel = FirstLevel;
    levelClearBonus = 0;
    currentMapPath = DefaultMapPath;
    nextMapPath.clear();
    levelName.clear();
}

}
