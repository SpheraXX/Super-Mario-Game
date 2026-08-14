#include "View/UI/UIButton.h"

#include "Controller/AppEngine.h"

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

// ── IClickable ────────────────────────────────────────────────────────────────

void UIButton::onHover(bool hovered) {
    isHovered = hovered;
    background.setFillColor(hovered ? colorHovered : colorNormal);
}

void UIButton::onClick() {
    if (onClickCallback) onClickCallback();
}

void UIButton::update(float deltaTime) {
    (void)deltaTime;
    // No scale animation: scaling the background causes sub-pixel text jitter at
    // 60 Hz (lerp + floor snaps to different pixels each frame). Color-only hover
    // feedback is stable and jitter-free. Scale effect can be re-added later.
    background.setPosition(pos);
    background.setSize(size);
}

bool UIButton::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        // Transform from window pixels to logical game coordinates before hit-test.
        const sf::Vector2f lp = controller::AppEngine::windowToLogical(moved->position);
        onHover(contains(lp.x, lp.y));
        return false;
    }

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressed->button == sf::Mouse::Button::Left) {
            const sf::Vector2f lp = controller::AppEngine::windowToLogical(pressed->position);
            if (contains(lp.x, lp.y)) {
                onClick();
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
    sfText.setFillColor(colorText);

    const sf::FloatRect bg  = background.getGlobalBounds();
    const sf::FloatRect lb  = sfText.getLocalBounds();
    const float tx = std::floor(bg.position.x + (bg.size.x - lb.size.x) / 2.f - lb.position.x);
    const float ty = std::floor(bg.position.y + (bg.size.y - lb.size.y) / 2.f - lb.position.y);
    sfText.setPosition({tx, ty});

    target.draw(sfText);
}

}  // namespace ui
}  // namespace view
