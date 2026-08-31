#include "View/UI/SolidButtonSkin.h"
#include "View/UI/UITheme.h"

namespace view {
namespace ui {

SolidButtonSkin::SolidButtonSkin() 
    : colorNormal(theme::ColorNormal), colorHovered(theme::ColorHovered) {
}

void SolidButtonSkin::setPosition(float x, float y) {
    background.setPosition({x, y});
}

void SolidButtonSkin::setSize(float w, float h) {
    background.setSize({w, h});
}

void SolidButtonSkin::updateState(bool hovered, bool enabled) {
    isHovered = hovered;
    isEnabled = enabled;
    
    if (!isEnabled) {
        if (keepTransparentWhenDisabled) {
            background.setFillColor(sf::Color::Transparent);
        } else {
            background.setFillColor(theme::ColorDisabled);
        }
    } else {
        background.setFillColor(isHovered ? colorHovered : colorNormal);
    }
}

void SolidButtonSkin::update(float deltaTime) {
    // Solid button has no animation
    (void)deltaTime;
}

void SolidButtonSkin::render(sf::RenderTarget& target) {
    target.draw(background);
}

void SolidButtonSkin::setColors(sf::Color normal, sf::Color hovered) {
    colorNormal = normal;
    colorHovered = hovered;
    updateState(isHovered, isEnabled);
}

void SolidButtonSkin::setKeepTransparentWhenDisabled(bool keep) {
    keepTransparentWhenDisabled = keep;
    updateState(isHovered, isEnabled);
}

} // namespace ui
} // namespace view
