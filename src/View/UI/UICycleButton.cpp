#include "View/UI/UICycleButton.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>

namespace view {
namespace ui {

UICycleButton::UICycleButton(const sf::Font& font, const std::string& label,
                             const std::vector<std::string>& opts, int idx,
                             sf::Vector2f position, sf::Vector2f sz)
    : fontPtr(&font), labelStr(label), options(opts)
    , currentIndex(std::clamp(idx, 0, static_cast<int>(opts.size()) - 1)) {
    pos  = position;
    size = sz;
    background.setFillColor(colorNormal);
    updateLayout();
}

void UICycleButton::updateLayout() {
    background.setPosition(pos);
    background.setSize(size);
}

void UICycleButton::setPosition(float x, float y) {
    UIElement::setPosition(x, y);
    updateLayout();
}

void UICycleButton::setSize(float w, float h) {
    UIElement::setSize(w, h);
    updateLayout();
}

void UICycleButton::setOptions(const std::vector<std::string>& opts, int idx) {
    options      = opts;
    currentIndex = std::clamp(idx, 0, static_cast<int>(opts.size()) - 1);
}

void UICycleButton::setIndex(int idx) {
    currentIndex = std::clamp(idx, 0, static_cast<int>(options.size()) - 1);
}

void UICycleButton::setColors(sf::Color normal, sf::Color hovered, sf::Color text) {
    colorNormal  = normal;
    colorHovered = hovered;
    colorText    = text;
    background.setFillColor(isHovered ? colorHovered : colorNormal);
}

void UICycleButton::onHover(bool h) {
    if (!enabled) return;
    isHovered = h;
    background.setFillColor(h ? colorHovered : colorNormal);
}

void UICycleButton::onClick() {
    if (options.empty()) return;
    currentIndex = (currentIndex + 1) % static_cast<int>(options.size());
    if (onChange) onChange(currentIndex);
    if (onClickCallback) onClickCallback();
}

bool UICycleButton::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    if (const auto* m = event.getIf<sf::Event::MouseMoved>()) {
        const sf::Vector2f lp = transformCoordinate(m->position);
        onHover(contains(lp.x, lp.y));
    }
    if (const auto* p = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (p->button == sf::Mouse::Button::Left) {
            const sf::Vector2f lp = transformCoordinate(p->position);
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

void UICycleButton::onMouseLeave() {
    onHover(false);
}

void UICycleButton::render(sf::RenderTarget& target) {
    if (!visible || !fontPtr) return;

    if (enabled) {
        background.setFillColor(isHovered ? colorHovered : colorNormal);
    } else {
        background.setFillColor(theme::ColorDisabled);
    }

    target.draw(background);

    sf::Color currentTextColor = enabled ? colorText : theme::ColorTextDisabled;

    sf::Text lbl(*fontPtr, labelStr, LabelSize);
    lbl.setFillColor(currentTextColor);
    const sf::FloatRect lb = lbl.getLocalBounds();
    lbl.setPosition({std::floor(pos.x + 4.f),
                     std::floor(pos.y + (size.y - lb.size.y) / 2.f - lb.position.y)});
    target.draw(lbl);

    if (!options.empty()) {
        const std::string display = "< " + options[currentIndex] + " >";
        sf::Text val(*fontPtr, display, ValueSize);
        val.setFillColor(currentTextColor);
        const sf::FloatRect vb = val.getLocalBounds();
        const float vx = std::floor(pos.x + size.x - vb.size.x - 6.f - vb.position.x);
        val.setPosition({vx, std::floor(pos.y + (size.y - vb.size.y) / 2.f - vb.position.y)});
        target.draw(val);
    }
}

}  // namespace ui
}  // namespace view
