#include "View/UI/UIButton.h"

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
    background.setPosition(pos);
    background.setSize(size);
    background.setFillColor(colorNormal);
}

void UIButton::setLabel(const std::string& text) { labelStr = text; }

void UIButton::setColors(sf::Color normal, sf::Color hovered, sf::Color text) {
    colorNormal  = normal;
    colorHovered = hovered;
    colorText    = text;
    background.setFillColor(isHovered ? colorHovered : colorNormal);
}

void UIButton::setFont(const sf::Font& font, unsigned int cs) {
    fontPtr  = &font;
    charSize = cs;
}

// ── UIElement overrides ──────────────────────────────────────────────────
void UIButton::setPosition(float x, float y) {
    UIElement::setPosition(x, y);
    background.setPosition(pos);
}

void UIButton::setSize(float w, float h) {
    UIElement::setSize(w, h);
    background.setSize(size);
}

// ── IClickable ────────────────────────────────────────────────────────────────

void UIButton::onHover(bool hovered) {
    if (!enabled) return;
    isHovered = hovered;
    background.setFillColor(hovered ? colorHovered : colorNormal);
}

void UIButton::onClick() {
    if (onClickCallback) onClickCallback();
}

void UIButton::onMouseLeave() {
    onHover(false);
}

void UIButton::update(float deltaTime) {
    (void)deltaTime;
    // No scale animation currently.
}

bool UIButton::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    // Fix Bug: Sticky Hover on MouseLeft
    if (event.is<sf::Event::MouseLeft>()) {
        onHover(false);
        return false;
    }

    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        // Transform from window pixels to logical game coordinates before hit-test.
        const sf::Vector2f lp = transformCoordinate(moved->position);
        bool inside = contains(lp.x, lp.y);
        onHover(inside);
        return inside; // Fix Bug: Event Penetration (Consume if inside)
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

    target.draw(background);

    if (!fontPtr || labelStr.empty()) return;

    // Build sf::Text at render-time (SFML 3: no default ctor).
    sf::Text sfText(*fontPtr, labelStr, charSize);
    
    if (enabled) {
        sfText.setFillColor(colorText);
        background.setFillColor(isHovered ? colorHovered : colorNormal);
    } else {
        sf::Color disabledColor(100, 100, 100);
        sf::Color disabledText(150, 150, 150);
        sfText.setFillColor(disabledText);
        background.setFillColor(disabledColor);
    }

    const sf::FloatRect bg  = background.getGlobalBounds();
    const sf::FloatRect lb  = sfText.getLocalBounds();
    const float tx = std::floor(bg.position.x + (bg.size.x - lb.size.x) / 2.f - lb.position.x);
    const float ty = std::floor(bg.position.y + (bg.size.y - lb.size.y) / 2.f - lb.position.y);
    sfText.setPosition({tx, ty});

    target.draw(sfText);
}

}  // namespace ui
}  // namespace view
