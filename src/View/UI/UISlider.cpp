#include "View/UI/UISlider.h"
#include "Controller/AppEngine.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <algorithm>
#include <cmath>
#include <string>

namespace view {
namespace ui {

UISlider::UISlider(const sf::Font& font, const std::string& label,
                   int min, int max, int step, int initial,
                   sf::Vector2f position, sf::Vector2f sz)
    : fontPtr(&font), labelStr(label)
    , minVal(min), maxVal(max), stepVal(step), value(initial) {
    pos  = position;
    size = sz;

    track.setFillColor(colorTrack);
    fill.setFillColor(colorFill);
    knob.setFillColor(colorKnob);

    updateLayout();
}

void UISlider::updateLayout() {
    const float labelW = labelStr.empty() ? 0.f : size.x * 0.45f;
    const float valTextW = 35.f; // reserve space for text
    const float trackH = std::floor(size.y * 0.3f);
    const float trackY = std::floor(pos.y + (size.y - trackH) / 2.f);
    const float trackX = std::floor(pos.x + labelW);
    const float trackW = std::max(10.f, size.x - labelW - valTextW);

    track.setPosition({trackX, trackY});
    track.setSize({trackW, trackH});

    fill.setPosition({trackX, trackY});
    
    const float kH = size.y * 0.7f;
    knob.setSize({KnobW, kH});

    clampAndSnap();
}

void UISlider::setPosition(float x, float y) {
    UIElement::setPosition(x, y);
    updateLayout();
}

void UISlider::setSize(float w, float h) {
    UIElement::setSize(w, h);
    updateLayout();
}

void UISlider::setColors(sf::Color t, sf::Color f, sf::Color k, sf::Color tx) {
    colorTrack = t; colorFill = f; colorKnob = k; colorText = tx;
    track.setFillColor(t); fill.setFillColor(f); knob.setFillColor(k);
}

void UISlider::setValue(int v) {
    value = v;
    clampAndSnap();
}

void UISlider::clampAndSnap() {
    value = std::clamp(value, minVal, maxVal);
    if (stepVal > 0) {
        value = minVal + ((value - minVal + stepVal / 2) / stepVal) * stepVal;
        value = std::clamp(value, minVal, maxVal);
    }
    const float kx = knobX();
    const sf::FloatRect tr = track.getGlobalBounds();
    fill.setSize({kx - tr.position.x, track.getSize().y});

    const float ky = std::floor(pos.y + (size.y - knob.getSize().y) / 2.f);
    knob.setPosition({std::floor(kx - KnobW / 2.f), ky});
}

float UISlider::knobX() const {
    const sf::FloatRect tr = track.getGlobalBounds();
    const float range = static_cast<float>(maxVal - minVal);
    if (range <= 0.f) return tr.position.x;
    const float t = static_cast<float>(value - minVal) / range;
    return tr.position.x + t * tr.size.x;
}

bool UISlider::handleEvent(const sf::Event& event) {
    if (!visible) return false;

    auto hitTrack = [&](sf::Vector2i wp) {
        const sf::Vector2f lp = controller::AppEngine::windowToLogical(wp);
        const sf::FloatRect tr = track.getGlobalBounds();
        return lp.x >= tr.position.x && lp.x <= tr.position.x + tr.size.x &&
               lp.y >= pos.y && lp.y <= pos.y + size.y;
    };

    auto setFromX = [&](float wx) {
        const sf::FloatRect tr = track.getGlobalBounds();
        const sf::Vector2f lp = controller::AppEngine::windowToLogical({static_cast<int>(wx), 0});
        const float t = std::clamp((lp.x - tr.position.x) / tr.size.x, 0.f, 1.f);
        const int newVal = minVal + static_cast<int>(std::round(t * (maxVal - minVal)));
        const int oldVal = value;
        setValue(newVal);
        if (value != oldVal && onChange) onChange(value);
    };

    if (const auto* p = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (p->button == sf::Mouse::Button::Left && hitTrack(p->position)) {
            dragging = true;
            setFromX(static_cast<float>(p->position.x));
            return true;
        }
    }
    if (const auto* r = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (r->button == sf::Mouse::Button::Left && dragging) {
            dragging = false;
        }
    }
    if (const auto* m = event.getIf<sf::Event::MouseMoved>()) {
        if (dragging) {
            setFromX(static_cast<float>(m->position.x));
        }
    }
    return false;
}

void UISlider::render(sf::RenderTarget& target) {
    if (!visible) return;

    target.draw(track);
    target.draw(fill);
    target.draw(knob);

    if (!fontPtr) return;

    sf::Text lbl(*fontPtr, labelStr, LabelSize);
    lbl.setFillColor(colorText);
    const sf::FloatRect lb = lbl.getLocalBounds();
    lbl.setPosition({std::floor(pos.x),
                     std::floor(pos.y + (size.y - lb.size.y) / 2.f - lb.position.y)});
    target.draw(lbl);

    const std::string valStr = std::to_string(value) + "%";
    sf::Text val(*fontPtr, valStr, LabelSize);
    val.setFillColor(colorText);
    const sf::FloatRect vb = val.getLocalBounds();
    
    const float labelW = labelStr.empty() ? 0.f : size.x * 0.45f;
    const float valTextW = 35.f;
    const float trackW = std::max(10.f, size.x - labelW - valTextW);
    const float valX = std::floor(pos.x + labelW + trackW + 5.f);

    val.setPosition({valX, std::floor(pos.y + (size.y - vb.size.y) / 2.f - vb.position.y)});
    target.draw(val);
}

}  // namespace ui
}  // namespace view
