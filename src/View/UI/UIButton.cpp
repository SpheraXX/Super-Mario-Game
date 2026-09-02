#include "View/UI/UIButton.h"
#include "View/UI/SolidButtonSkin.h"
#include "View/UI/NineSliceButtonSkin.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>

namespace view {
namespace ui {

UIButton::UIButton(const sf::Font& font, const std::string& label,
                   unsigned int cs, sf::Vector2f position, sf::Vector2f sz)
    : fontPtr(&font), labelStr(label), charSize(cs) {
    pos  = position;
    size = sz;
    
    // Default to Elegant UI NineSlice skin
    skin = std::make_unique<NineSliceButtonSkin>("elegant_panel");
    skin->setPosition(pos.x, pos.y);
    skin->setSize(size.x, size.y);
    skin->setColors(sf::Color::White, sf::Color::White);
}

void UIButton::setSkin(std::unique_ptr<IButtonSkin> newSkin) {
    skin = std::move(newSkin);
    if (skin) {
        skin->setPosition(pos.x, pos.y);
        skin->setSize(size.x, size.y);
        skin->updateState(isHovered, enabled);
    }
}

void UIButton::setMaskMode(bool enable) {
    if (skin) skin->setKeepTransparentWhenDisabled(enable);
}

void UIButton::setLabel(const std::string& text) { labelStr = text; }

void UIButton::setColors(sf::Color normal, sf::Color hovered, sf::Color text) {
    colorText = text;
    if (skin) {
        skin->setColors(normal, hovered);
    }
}

void UIButton::setFont(const sf::Font& font, unsigned int cs) {
    fontPtr  = &font;
    charSize = cs;
}

void UIButton::setPosition(float x, float y) {
    UIElement::setPosition(x, y);
    if (skin) skin->setPosition(x, y);
}

void UIButton::setSize(float w, float h) {
    UIElement::setSize(w, h);
    if (skin) skin->setSize(w, h);
}

void UIButton::setEnabled(bool e) {
    UIElement::setEnabled(e);
    if (skin) skin->updateState(isHovered, enabled);
}

void UIButton::onHover(bool hovered) {
    if (!enabled) return;
    isHovered = hovered;
    if (skin) skin->updateState(isHovered, enabled);
}

void UIButton::onClick() {
    if (onClickCallback) onClickCallback();
}

void UIButton::onMouseLeave() {
    onHover(false);
}

void UIButton::update(float deltaTime) {
    if (skin) skin->update(deltaTime);
}

bool UIButton::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    if (event.is<sf::Event::MouseLeft>()) {
        onHover(false);
        return false;
    }

    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        const sf::Vector2f lp = transformCoordinate(moved->position);
        bool inside = contains(lp.x, lp.y);
        onHover(inside);
        return inside; 
    }

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressed->button == sf::Mouse::Button::Left) {
            const sf::Vector2f lp = transformCoordinate(pressed->position);
            if (contains(lp.x, lp.y)) {
                if (enabled) {
                    onClick();
                }
                return true;
            }
        }
    }

    return false;
}

void UIButton::render(sf::RenderTarget& target) {
    if (!visible) return;

    if (skin) skin->render(target);

    if (!fontPtr || labelStr.empty()) return;

    sf::Text sfText(*fontPtr, labelStr, charSize);
    
    if (enabled) {
        sfText.setFillColor(colorText);
    } else {
        sfText.setFillColor(theme::ColorTextDisabled);
    }

    const sf::FloatRect lb  = sfText.getLocalBounds();
    const float tx = std::floor(pos.x + (size.x - lb.size.x) / 2.f - lb.position.x);
    const float ty = std::floor(pos.y + (size.y - lb.size.y) / 2.f - lb.position.y);
    sfText.setPosition({tx, ty});

    target.draw(sfText);
}

}  // namespace ui
}  // namespace view
