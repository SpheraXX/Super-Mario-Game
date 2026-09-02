#include "View/UI/UIConfigManager.h"
#include "Model/Core/LogManager.h"
#include "ext/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace view {
namespace ui {

UIConfigManager& UIConfigManager::instance() {
    static UIConfigManager inst;
    return inst;
}

UIConfigManager::UIConfigManager() {
    // Attempt to load on startup
    load();
}

bool UIConfigManager::load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        model::LogManager::instance().error("[UIConfigManager] Could not open " + filepath);
        return false;
    }

    try {
        json j;
        file >> j;

        configs.clear();
        for (auto& [key, value] : j.items()) {
            UIConfig config;
            config.texturePath = value.value("texture", "");
            
            if (value.contains("rect_normal")) {
                auto arr = value["rect_normal"];
                config.rectNormal = sf::IntRect({arr[0], arr[1]}, {arr[2], arr[3]});
            }
            if (value.contains("rect_hovered")) {
                auto arr = value["rect_hovered"];
                config.rectHovered = sf::IntRect({arr[0], arr[1]}, {arr[2], arr[3]});
            } else {
                config.rectHovered = config.rectNormal;
            }
            
            if (value.contains("rect_pressed")) {
                auto arr = value["rect_pressed"];
                config.rectPressed = sf::IntRect({arr[0], arr[1]}, {arr[2], arr[3]});
            } else {
                config.rectPressed = config.rectHovered;
            }
            
            if (value.contains("rect_disabled")) {
                auto arr = value["rect_disabled"];
                config.rectDisabled = sf::IntRect({arr[0], arr[1]}, {arr[2], arr[3]});
            } else {
                config.rectDisabled = config.rectNormal;
            }

            if (value.contains("margin")) {
                auto arr = value["margin"];
                config.margin[0] = arr[0]; // Left
                config.margin[1] = arr[1]; // Right
                config.margin[2] = arr[2]; // Top
                config.margin[3] = arr[3]; // Bottom
            }
            
            configs[key] = config;
        }
        model::LogManager::instance().info("[UIConfigManager] Successfully loaded " + std::to_string(configs.size()) + " configs from " + filepath);
        // Notify all registered skins to reload their texture/rect
        for (auto& [id, cb] : m_reloadCallbacks) cb();
        return true;
    } catch (const json::exception& e) {
        model::LogManager::instance().error(std::string("[UIConfigManager] JSON parse error in ") + filepath + ": " + e.what());
        return false;
    }
}

const UIConfig* UIConfigManager::getConfig(const std::string& id) const {
    auto it = configs.find(id);
    if (it != configs.end()) {
        return &it->second;
    }
    return nullptr;
}

int UIConfigManager::registerReloadCallback(std::function<void()> cb) {
    int id = m_nextCallbackId++;
    m_reloadCallbacks.emplace_back(id, std::move(cb));
    return id;
}

void UIConfigManager::unregisterReloadCallback(int id) {
    m_reloadCallbacks.erase(
        std::remove_if(m_reloadCallbacks.begin(), m_reloadCallbacks.end(),
                       [id](const auto& p){ return p.first == id; }),
        m_reloadCallbacks.end());
}

} // namespace ui
} // namespace view
