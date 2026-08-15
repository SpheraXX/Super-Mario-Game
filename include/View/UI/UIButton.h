#ifndef VIEW_UI_UIBUTTON_H
#define VIEW_UI_UIBUTTON_H

#include "View/UI/UIElement.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <functional>
#include <string>

namespace view {
namespace ui {

// An interactive button: text label centred on a coloured rectangle.
//
// SFML 3 has no sf::Text default constructor, so the label string + font
// metadata are stored as plain POD/strings and a temporary sf::Text is built
// in render(). The RectangleShape IS default-constructible, so it stays
// as a member.
//
// Command Pattern: the caller injects a std::function<void()> callback.
// UIButton never includes any game-logic header — DIP is preserved.
class UIButton : public UIElement, public IClickable {
public:
    // Default-constructible — configure via setters before first render.
    UIButton() = default;

    UIButton(const sf::Font& font, const std::string& labelStr,
             unsigned int charSize, sf::Vector2f position, sf::Vector2f size);

    // ── Command injection ────────────────────────────────────────────────────
    void setOnClick(std::function<void()> callback) { onClickCallback = std::move(callback); }

    // ── Styling ──────────────────────────────────────────────────────────────
    void setLabel(const std::string& text);
    void setColors(sf::Color normal, sf::Color hovered, sf::Color text);
    void setFont(const sf::Font& font, unsigned int cs);

    // ── UIElement overrides ──────────────────────────────────────────────────
    void setPosition(float x, float y) override;
    void setSize(float w, float h) override;

    void render(sf::RenderTarget& target) override;
    bool handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void onMouseLeave() override;

    // ── IClickable overrides ─────────────────────────────────────────────────
    void onHover(bool hovered) override;
    void onClick() override;

private:
    sf::RectangleShape background;

    // Font metadata (sf::Text built at render-time — SFML 3 compat).
    const sf::Font* fontPtr   = nullptr;
    std::string     labelStr;
    unsigned int    charSize  = 8u;

    sf::Color colorNormal     = sf::Color(60,  60,  80);
    sf::Color colorHovered    = sf::Color(100, 100, 140);
    sf::Color colorText       = sf::Color::White;

    bool isHovered = false;

    std::function<void()> onClickCallback;
};

}  // namespace ui
}  // namespace view

#endif
