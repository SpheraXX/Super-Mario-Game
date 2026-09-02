#ifndef VIEW_UI_NINESLICEBUTTONSKIN_H
#define VIEW_UI_NINESLICEBUTTONSKIN_H

#include "View/UI/IButtonSkin.h"
#include "View/UI/NineSliceSprite.h"
#include "View/UI/UIConfigManager.h"
#include <string>

namespace view {
namespace ui {

class NineSliceButtonSkin : public IButtonSkin {
public:
    explicit NineSliceButtonSkin(const std::string& configId);
    ~NineSliceButtonSkin();

    // Reloads texture and rect from UIConfigManager (call after F5 hot-reload)
    void reloadFromConfig();

    void setPosition(float x, float y) override;
    void setSize(float w, float h) override;
    void updateState(bool hovered, bool enabled) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

    void setColors(sf::Color normal, sf::Color hovered) override;
    void setKeepTransparentWhenDisabled(bool keep) override;

private:
    NineSliceSprite m_sprite;
    UIConfig m_config;
    std::string m_configId; // kept for hot-reload
    int m_reloadCallbackId = -1; // ID returned by UIConfigManager::registerReloadCallback
    
    sf::Color m_colorNormal;
    sf::Color m_colorHovered;
    bool m_keepTransparent;
    
    bool m_isHovered;
    bool m_isEnabled;
};

} // namespace ui
} // namespace view

#endif
