#ifndef VIEW_UI_UICYCLEBUTTON_H
#define VIEW_UI_UICYCLEBUTTON_H

#include "View/UI/UIElement.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "View/UI/UITheme.h"

#include <functional>
#include <string>
#include <vector>

namespace view {
namespace ui {

class UICycleButton : public UIElement, public IClickable {
public:
    UICycleButton() = default;

    UICycleButton(const sf::Font& font, const std::string& label,
                  const std::vector<std::string>& options, int initialIndex,
                  sf::Vector2f position, sf::Vector2f size);

    void setOnChange(std::function<void(int)> cb) { onChange = std::move(cb); }
    void setOnClick(std::function<void()> cb)    { onClickCallback = std::move(cb); }

    int         getIndex()  const { return currentIndex; }
    const std::string& current() const { return options[currentIndex]; }

    void setOptions(const std::vector<std::string>& opts, int idx = 0);
    void setIndex(int idx);
    void setColors(sf::Color normal, sf::Color hovered, sf::Color text);
    void setFont(const sf::Font& font, unsigned int cs);

    void setPosition(float x, float y) override;
    void setSize(float w, float h) override;

    // ── UIElement overrides ──────────────────────────────────────────────────
    void render(sf::RenderTarget& target) override;
    bool handleEvent(const sf::Event& event) override;

    // ── IClickable overrides ─────────────────────────────────────────────────
    void onHover(bool hovered) override;
    void onMouseLeave() override;
    void onClick() override;

private:
    void updateLayout();

    const sf::Font*           fontPtr      = nullptr;
    std::string               labelStr;
    std::vector<std::string>  options;
    int                       currentIndex = 0;

    sf::RectangleShape        background;
    bool                      isHovered    = false;

    sf::Color colorNormal  = theme::CycleNormal;
    sf::Color colorHovered = theme::CycleHovered;
    sf::Color colorText    = theme::ColorText;

    static constexpr unsigned int LabelSize  = 6u;
    static constexpr unsigned int ValueSize  = 6u;

    std::function<void(int)> onChange;
    std::function<void()>    onClickCallback;
};

}  // namespace ui
}  // namespace view

#endif
