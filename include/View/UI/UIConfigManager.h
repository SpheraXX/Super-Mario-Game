#ifndef VIEW_UI_UICONFIGMANAGER_H
#define VIEW_UI_UICONFIGMANAGER_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <SFML/Graphics/Rect.hpp>

namespace view {
namespace ui {

struct UIConfig {
    std::string texturePath;
    sf::IntRect rectNormal;
    sf::IntRect rectHovered;
    sf::IntRect rectPressed;
    sf::IntRect rectDisabled;
    int margin[4] = {0, 0, 0, 0}; // Left, Right, Top, Bottom (pixels)
};

class UIConfigManager {
public:
    static UIConfigManager& instance();
    
    // Loads or reloads the config from assets/data/ui_assets.json.
    // After a successful load, all registered onReload callbacks are fired.
    bool load(const std::string& filepath = "assets/data/ui_assets.json");
    
    // Gets config by ID, returns nullptr if not found
    const UIConfig* getConfig(const std::string& id) const;

    // Register a callback invoked every time load() succeeds (for hot-reload).
    // Returns a registration ID that can be passed to unregisterReloadCallback.
    int registerReloadCallback(std::function<void()> cb);
    void unregisterReloadCallback(int id);

private:
    UIConfigManager();
    ~UIConfigManager() = default;
    
    std::unordered_map<std::string, UIConfig> configs;

    // Hot-reload observers
    int m_nextCallbackId = 0;
    std::vector<std::pair<int, std::function<void()>>> m_reloadCallbacks;
};

} // namespace ui
} // namespace view

#endif
