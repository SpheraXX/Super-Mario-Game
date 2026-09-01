#include "Model/Core/WorldManager.h"
#include "Model/Core/LogManager.h"
#include "ext/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace model {

WorldManager& WorldManager::instance() {
    static WorldManager singleton;
    return singleton;
}

void WorldManager::load(const std::string& filepath) {
    m_worlds.clear();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        LogManager::instance().error("[WorldManager] Failed to open " + filepath);
        return;
    }

    try {
        json j;
        file >> j;

        if (j.contains("worlds") && j["worlds"].is_array()) {
            for (const auto& w : j["worlds"]) {
                WorldData wd;
                wd.id = w.value("id", "");
                wd.title = w.value("title", "");
                wd.previewImage = w.value("preview_image", "");
                wd.bgaImage = w.value("bga_image", "");

                if (w.contains("levels") && w["levels"].is_array()) {
                    for (const auto& l : w["levels"]) {
                        LevelData ld;
                        ld.id = l.value("id", "");
                        ld.mapPath = l.value("map_path", "");
                        wd.levels.push_back(ld);
                    }
                }
                m_worlds.push_back(wd);
            }
        }
        LogManager::instance().info("[WorldManager] Loaded " + std::to_string(m_worlds.size()) + " worlds.");
    } catch (const json::exception& e) {
        LogManager::instance().error(std::string("[WorldManager] JSON parse error: ") + e.what());
    }
}

const std::vector<WorldData>& WorldManager::getWorlds() const {
    return m_worlds;
}

const WorldData* WorldManager::getWorld(const std::string& id) const {
    for (const auto& w : m_worlds) {
        if (w.id == id) {
            return &w;
        }
    }
    return nullptr;
}

} // namespace model
