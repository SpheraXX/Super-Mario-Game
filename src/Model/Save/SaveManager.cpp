#include "Model/Save/SaveManager.h"
#include "Model/Save/ProfileManager.h"
#include "Model/Core/LogManager.h"
#include "ext/json.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace model {

SaveManager& SaveManager::instance() {
    static SaveManager singleton;
    return singleton;
}

std::string SaveManager::getActiveSavePath() const {
    int activeIndex = ProfileManager::instance().getActiveProfileIndex();
    return "assets/data/save_slot_" + std::to_string(activeIndex) + ".json";
}

bool SaveManager::hasSaveFile(const std::string& path) const {
    std::string actualPath = path.empty() ? getActiveSavePath() : path;
    std::error_code ec;
    return std::filesystem::exists(actualPath, ec) && std::filesystem::is_regular_file(actualPath, ec);
}

bool SaveManager::deleteSave(const std::string& path) const {
    std::string actualPath = path.empty() ? getActiveSavePath() : path;
    std::error_code ec;
    if (std::filesystem::exists(actualPath, ec)) {
        return std::filesystem::remove(actualPath, ec);
    }
    return true;
}

bool SaveManager::save(const GameSaveData& data, const std::string& path) const {
    std::string actualPath = path.empty() ? getActiveSavePath() : path;
    try {
        std::filesystem::create_directories(std::filesystem::path(actualPath).parent_path());

        json j;
        j["version"] = data.version;
        j["score"]   = data.score;
        j["lives"]   = data.lives;
        j["coins"]   = data.coins;

        j["unlocked_worlds"] = data.unlocked_worlds;
        j["level_progress"] = data.level_progress;
        j["high_scores"] = data.high_scores;

        // Level data
        j["level"]["currentLevel"]        = data.level.currentLevel;
        j["level"]["mapPath"]             = data.level.mapPath;
        j["level"]["nextMapPath"]         = data.level.nextMapPath;
        j["level"]["levelName"]           = data.level.levelName;
        j["level"]["currentArea"]         = data.level.currentArea;
        j["level"]["remainingTime"]       = data.level.remainingTime;
        j["level"]["hasEntitiesSnapshot"] = data.level.hasEntitiesSnapshot;

        // Removed / broken tiles
        json removedArray = json::array();
        for (const auto& tile : data.level.removedTiles) {
            removedArray.push_back({{"row", tile.row}, {"col", tile.col}});
        }
        j["level"]["removedTiles"] = removedArray;

        // Coin blocks
        json coinBlocksArray = json::array();
        for (const auto& cb : data.level.coinBlocks) {
            coinBlocksArray.push_back({{"posX", cb.posX}, {"posY", cb.posY}, {"opened", cb.opened}});
        }
        j["level"]["coinBlocks"] = coinBlocksArray;

        // Enemies
        json enemiesArray = json::array();
        for (const auto& e : data.level.enemies) {
            enemiesArray.push_back({
                {"type", e.type},
                {"posX", e.posX},
                {"posY", e.posY},
                {"velX", e.velX},
                {"velY", e.velY},
                {"isDormant", e.isDormant},
                {"facingRight", e.facingRight},
                {"direction", e.direction},
                {"isWinged", e.isWinged},
                {"state", e.state}
            });
        }
        j["level"]["enemies"] = enemiesArray;

        // Items
        json itemsArray = json::array();
        for (const auto& it : data.level.items) {
            itemsArray.push_back({
                {"type", it.type},
                {"posX", it.posX},
                {"posY", it.posY},
                {"velX", it.velX},
                {"velY", it.velY},
                {"direction", it.direction}
            });
        }
        j["level"]["items"] = itemsArray;

        // Player data
        j["player"]["posX"]            = data.player.posX;
        j["player"]["posY"]            = data.player.posY;
        j["player"]["velX"]            = data.player.velX;
        j["player"]["velY"]            = data.player.velY;
        j["player"]["isBig"]           = data.player.isBig;
        j["player"]["power"]           = data.player.power;
        j["player"]["starDuration"]    = data.player.starDuration;
        j["player"]["facingDirection"] = data.player.facingDirection;
        j["player"]["isLuigi"]         = data.player.isLuigi;

        std::ofstream file(actualPath);
        if (!file.is_open()) {
            LogManager::instance().error("[SaveManager] Failed to write save file: " + actualPath);
            return false;
        }

        file << j.dump(4);
        LogManager::instance().info("[SaveManager] Game saved to: " + actualPath);
        return true;
    } catch (const std::exception& e) {
        LogManager::instance().error(std::string("[SaveManager] Error saving game: ") + e.what());
        return false;
    }
}

bool SaveManager::load(GameSaveData& outData, const std::string& path) const {
    std::string actualPath = path.empty() ? getActiveSavePath() : path;
    if (!hasSaveFile(actualPath)) {
        LogManager::instance().warning("[SaveManager] Save file not found: " + actualPath);
        return false;
    }

    try {
        std::ifstream file(actualPath);
        if (!file.is_open()) {
            LogManager::instance().error("[SaveManager] Failed to read save file: " + actualPath);
            return false;
        }

        json j;
        file >> j;

        outData.version = j.value("version", 1);
        outData.score   = j.value("score", 0);
        outData.lives   = j.value("lives", 10);
        outData.coins   = j.value("coins", 0);

        if (j.contains("unlocked_worlds") && j["unlocked_worlds"].is_array()) {
            outData.unlocked_worlds = j["unlocked_worlds"].get<std::vector<std::string>>();
        }
        if (j.contains("level_progress") && j["level_progress"].is_object()) {
            outData.level_progress = j["level_progress"].get<std::unordered_map<std::string, std::string>>();
        }
        if (j.contains("high_scores") && j["high_scores"].is_object()) {
            outData.high_scores = j["high_scores"].get<std::unordered_map<std::string, int>>();
        }

        if (j.contains("level") && j["level"].is_object()) {
            const auto& lvl = j["level"];
            outData.level.currentLevel        = lvl.value("currentLevel", 1);
            outData.level.mapPath             = lvl.value("mapPath", "assets/maps/debug.map");
            outData.level.nextMapPath         = lvl.value("nextMapPath", "");
            outData.level.levelName           = lvl.value("levelName", "");
            outData.level.currentArea         = lvl.value("currentArea", std::size_t{0});
            outData.level.remainingTime       = lvl.value("remainingTime", 400.0f);
            outData.level.hasEntitiesSnapshot = lvl.value("hasEntitiesSnapshot", false);

            outData.level.removedTiles.clear();
            if (lvl.contains("removedTiles") && lvl["removedTiles"].is_array()) {
                for (const auto& item : lvl["removedTiles"]) {
                    TileCoord tc;
                    tc.row = item.value("row", std::size_t{0});
                    tc.col = item.value("col", std::size_t{0});
                    outData.level.removedTiles.push_back(tc);
                }
            }

            outData.level.coinBlocks.clear();
            if (lvl.contains("coinBlocks") && lvl["coinBlocks"].is_array()) {
                for (const auto& item : lvl["coinBlocks"]) {
                    BlockSaveData bsd;
                    bsd.posX   = item.value("posX", 0.0f);
                    bsd.posY   = item.value("posY", 0.0f);
                    bsd.opened = item.value("opened", false);
                    outData.level.coinBlocks.push_back(bsd);
                }
            }

            outData.level.enemies.clear();
            if (lvl.contains("enemies") && lvl["enemies"].is_array()) {
                for (const auto& item : lvl["enemies"]) {
                    EnemySaveData esd;
                    esd.type        = item.value("type", "Goomba");
                    esd.posX        = item.value("posX", 0.0f);
                    esd.posY        = item.value("posY", 0.0f);
                    esd.velX        = item.value("velX", 0.0f);
                    esd.velY        = item.value("velY", 0.0f);
                    esd.isDormant   = item.value("isDormant", false);
                    esd.facingRight = item.value("facingRight", true);
                    esd.direction   = item.value("direction", 1);
                    esd.isWinged    = item.value("isWinged", false);
                    esd.state       = item.value("state", "Walking");
                    outData.level.enemies.push_back(esd);
                }
            }

            outData.level.items.clear();
            if (lvl.contains("items") && lvl["items"].is_array()) {
                for (const auto& item : lvl["items"]) {
                    ItemSaveData isd;
                    isd.type      = item.value("type", "Mushroom");
                    isd.posX      = item.value("posX", 0.0f);
                    isd.posY      = item.value("posY", 0.0f);
                    isd.velX      = item.value("velX", 0.0f);
                    isd.velY      = item.value("velY", 0.0f);
                    isd.direction = item.value("direction", 1);
                    outData.level.items.push_back(isd);
                }
            }
        }

        if (j.contains("player") && j["player"].is_object()) {
            const auto& ply = j["player"];
            outData.player.posX            = ply.value("posX", 0.0f);
            outData.player.posY            = ply.value("posY", 0.0f);
            outData.player.velX            = ply.value("velX", 0.0f);
            outData.player.velY            = ply.value("velY", 0.0f);
            outData.player.isBig           = ply.value("isBig", false);
            outData.player.power           = ply.value("power", "None");
            outData.player.starDuration    = ply.value("starDuration", 0.0f);
            outData.player.facingDirection = ply.value("facingDirection", 1);
            outData.player.isLuigi         = ply.value("isLuigi", false);
        }

        LogManager::instance().info("[SaveManager] Game loaded from: " + actualPath);
        return true;
    } catch (const std::exception& e) {
        LogManager::instance().error(std::string("[SaveManager] JSON parse error while loading save: ") + e.what());
        return false;
    }
}

} // namespace model
