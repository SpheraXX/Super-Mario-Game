#include "Model/SettingsManager.h"
#include "ext/json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace model {

SettingsManager& SettingsManager::instance() {
    static SettingsManager singleton;
    return singleton;
}

SettingsManager::SettingsManager() {
    load();
}

void SettingsManager::apply(const Settings& next) {
    if (current != next) {
        current = next;
        save();
        for (auto observer : observers) {
            observer->onSettingsChanged(current);
        }
    }
}

void SettingsManager::addObserver(ISettingsObserver* observer) {
    if (observer) {
        observers.push_back(observer);
        observer->onSettingsChanged(current); // Initial sync
    }
}

void SettingsManager::removeObserver(ISettingsObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void SettingsManager::resetToDefaults() {
    apply(Settings::defaults());
}

namespace {
std::string langToStr(Language l) {
    return l == Language::Vietnamese ? "vi" : "en";
}
Language strToLang(const std::string& s) {
    return s == "vi" ? Language::Vietnamese : Language::English;
}
}

void SettingsManager::load() {
    current = Settings::defaults();

    std::ifstream file(FilePath);
    if (!file.is_open()) {
        std::cerr << "[SettingsManager] settings.json not found, creating with defaults.\n";
        save();
        return;
    }

    try {
        json j;
        file >> j;

        if (j.contains("graphics")) {
            const auto& g = j["graphics"];
            if (g.contains("fullscreen"))      current.fullscreen      = g["fullscreen"].get<bool>();
            if (g.contains("ratio"))           current.ratio           = static_cast<model::AspectRatio>(g["ratio"].get<int>());
            if (g.contains("resolutionIndex")) current.resolutionIndex = g["resolutionIndex"].get<int>();
            if (g.contains("quality"))         current.quality         = static_cast<model::GraphicsQuality>(g["quality"].get<int>());
            if (g.contains("vsync"))           current.vsync           = g["vsync"].get<bool>();
        }

        if (j.contains("sound")) {
            const auto& s = j["sound"];
            if (s.contains("masterVolume")) current.masterVolume = s["masterVolume"].get<int>();
            if (s.contains("musicVolume"))  current.musicVolume  = s["musicVolume"].get<int>();
            if (s.contains("sfxVolume"))    current.sfxVolume    = s["sfxVolume"].get<int>();
        }

        if (j.contains("language")) {
            current.language = strToLang(j["language"].get<std::string>());
        }

        if (j.contains("controls")) {
            const auto& c = j["controls"];
            if (c.contains("moveLeft"))   current.keyMoveLeft  = c["moveLeft"].get<int>();
            if (c.contains("moveRight"))  current.keyMoveRight = c["moveRight"].get<int>();
            if (c.contains("jump"))       current.keyJump      = c["jump"].get<int>();
            if (c.contains("run"))        current.keyRun       = c["run"].get<int>();
            if (c.contains("pause"))      current.keyPause     = c["pause"].get<int>();
            if (c.contains("cycleDisplay")) current.keyCycleDisplay = c["cycleDisplay"].get<int>();
            if (c.contains("back"))       current.keyBack      = c["back"].get<int>();
            if (c.contains("slot"))       current.controlSlot  = c["slot"].get<int>();
        }

    } catch (const json::exception& e) {
        std::cerr << "[SettingsManager] JSON parse error: " << e.what() << "\n";
        current = Settings::defaults();
        save();
    }
}

void SettingsManager::save() const {
    std::filesystem::create_directories(
        std::filesystem::path(FilePath).parent_path());

    json j;
    j["graphics"]["fullscreen"]      = current.fullscreen;
    j["graphics"]["ratio"]           = static_cast<int>(current.ratio);
    j["graphics"]["resolutionIndex"] = current.resolutionIndex;
    j["graphics"]["quality"]         = static_cast<int>(current.quality);
    j["graphics"]["vsync"]           = current.vsync;

    j["sound"]["masterVolume"]    = current.masterVolume;
    j["sound"]["musicVolume"]     = current.musicVolume;
    j["sound"]["sfxVolume"]       = current.sfxVolume;

    j["language"]                 = langToStr(current.language);

    j["controls"]["moveLeft"]     = current.keyMoveLeft;
    j["controls"]["moveRight"]    = current.keyMoveRight;
    j["controls"]["jump"]         = current.keyJump;
    j["controls"]["run"]          = current.keyRun;
    j["controls"]["pause"]        = current.keyPause;
    j["controls"]["cycleDisplay"] = current.keyCycleDisplay;
    j["controls"]["back"]         = current.keyBack;
    j["controls"]["slot"]         = current.controlSlot;

    std::ofstream file(FilePath);
    if (!file.is_open()) return;
    file << j.dump(4);
}

}  // namespace model
