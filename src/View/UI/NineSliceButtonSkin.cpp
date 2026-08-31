#include "View/UI/NineSliceButtonSkin.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"
#include <iostream>

namespace view {
namespace ui {

NineSliceButtonSkin::NineSliceButtonSkin(const std::string& configId)
    : m_configId(configId)
    , m_colorNormal(sf::Color::White)
    , m_colorHovered(sf::Color(200, 200, 255)) // slight blue tint for hover feedback
    , m_keepTransparent(false)
    , m_isHovered(false)
    , m_isEnabled(true) {
    
    reloadFromConfig();

    // Register for automatic hot-reload when F5 is pressed.
    // The lambda captures 'this' by pointer — safe because the skin outlives the callback,
    // and we unregister in the destructor before the skin is destroyed.
    m_reloadCallbackId = UIConfigManager::instance().registerReloadCallback(
        [this]() { reloadFromConfig(); });
}

NineSliceButtonSkin::~NineSliceButtonSkin() {
    if (m_reloadCallbackId >= 0) {
        UIConfigManager::instance().unregisterReloadCallback(m_reloadCallbackId);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Hot-reload support: re-read config and re-bind texture.
// Safe to call multiple times (e.g. on F5).
// ─────────────────────────────────────────────────────────────────────────────
void NineSliceButtonSkin::reloadFromConfig() {
    const UIConfig* config = UIConfigManager::instance().getConfig(m_configId);
    if (!config) {
        std::cerr << "[NineSliceButtonSkin] Config ID not found: " << m_configId << "\n";
        return;
    }
    m_config = *config;

    // Re-bind texture — AssetManager always returns the same (or reloaded) reference,
    // so this fixes the dangling-pointer issue after reloadAll().
    const sf::Texture& tex = AssetManager::instance().getTexture(m_config.texturePath);
    m_sprite.setTexture(tex);

    m_sprite.setMargins(
        static_cast<float>(m_config.margin[0]),
        static_cast<float>(m_config.margin[1]),
        static_cast<float>(m_config.margin[2]),
        static_cast<float>(m_config.margin[3])
    );

    // Restore the correct rect for current visual state
    updateState(m_isHovered, m_isEnabled);
}

void NineSliceButtonSkin::setPosition(float x, float y) {
    m_sprite.setPosition(x, y);
}

void NineSliceButtonSkin::setSize(float w, float h) {
    m_sprite.setSize(w, h);
}

void NineSliceButtonSkin::updateState(bool hovered, bool enabled) {
    m_isHovered = hovered;
    m_isEnabled = enabled;

    if (!enabled) {
        if (m_keepTransparent) {
            m_sprite.setColor(sf::Color::Transparent);
        } else {
            m_sprite.setColor(theme::ColorDisabled);
        }
        m_sprite.setTextureRect(m_config.rectDisabled);
    } else if (hovered) {
        m_sprite.setColor(m_colorHovered);
        m_sprite.setTextureRect(m_config.rectHovered);
    } else {
        m_sprite.setColor(m_colorNormal);
        m_sprite.setTextureRect(m_config.rectNormal);
    }
}

void NineSliceButtonSkin::update(float /*deltaTime*/) {
    // Reserved for future pulse/animation effects
}

void NineSliceButtonSkin::render(sf::RenderTarget& target) {
    if (!m_isEnabled && m_keepTransparent) return;
    m_sprite.render(target);
}

void NineSliceButtonSkin::setColors(sf::Color normal, sf::Color hovered) {
    m_colorNormal  = normal;
    m_colorHovered = hovered;
    updateState(m_isHovered, m_isEnabled);
}

void NineSliceButtonSkin::setKeepTransparentWhenDisabled(bool keep) {
    m_keepTransparent = keep;
    updateState(m_isHovered, m_isEnabled);
}

} // namespace ui
} // namespace view
