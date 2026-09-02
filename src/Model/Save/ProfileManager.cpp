#include "Model/Save/ProfileManager.h"
#include "Model/Core/LogManager.h"
#include "ext/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace model {

ProfileManager& ProfileManager::instance() {
    static ProfileManager singleton;
    return singleton;
}

ProfileManager::ProfileManager() {
    profiles.resize(4);
}

void ProfileManager::load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LogManager::instance().warning("[ProfileManager] No profile data found. Creating 3 test profiles.");
        profiles[0] = {"PLAYER 1", 0, 0, false};
        profiles[1] = {"PLAYER 2", 0, 0, false};
        profiles[2] = {"PLAYER 3", 0, 0, false};
        activeProfileIndex = 0;
        save(path);
        return;
    }

    try {
        json j;
        file >> j;

        activeProfileIndex = j.value("active_profile_index", 0);

        if (j.contains("slots") && j["slots"].is_array()) {
            int i = 0;
            for (const auto& slot : j["slots"]) {
                if (i >= 4) break;
                profiles[i].name = slot.value("name", "EMPTY");
                profiles[i].total_score = slot.value("total_score", 0);
                profiles[i].passed_levels = slot.value("passed_levels", 0);
                profiles[i].is_empty = slot.value("is_empty", true);
                i++;
            }
        }
    } catch (const json::exception& e) {
        LogManager::instance().error(std::string("[ProfileManager] JSON parse error: ") + e.what());
    }
}

void ProfileManager::save(const std::string& path) const {
    json j;
    j["active_profile_index"] = activeProfileIndex;
    
    json slots = json::array();
    for (int i = 0; i < 4; ++i) {
        json slot;
        slot["name"] = profiles[i].name;
        slot["total_score"] = profiles[i].total_score;
        slot["passed_levels"] = profiles[i].passed_levels;
        slot["is_empty"] = profiles[i].is_empty;
        slots.push_back(slot);
    }
    j["slots"] = slots;

    std::ofstream file(path);
    if (file.is_open()) {
        file << j.dump(4);
    } else {
        LogManager::instance().error("[ProfileManager] Failed to write profile data to " + path);
    }
}

const std::vector<Profile>& ProfileManager::getProfiles() const {
    return profiles;
}

void ProfileManager::updateProfile(int index, const Profile& profile) {
    if (index >= 0 && index < 4) {
        profiles[index] = profile;
        save();
    }
}

void ProfileManager::deleteProfile(int index) {
    if (index >= 0 && index < 4) {
        profiles[index] = Profile(); // Reset to default empty
        save();
    }
}

int ProfileManager::getActiveProfileIndex() const {
    return activeProfileIndex;
}

void ProfileManager::setActiveProfileIndex(int index) {
    if (index >= 0 && index < 4) {
        activeProfileIndex = index;
        save();
    }
}

} // namespace model
