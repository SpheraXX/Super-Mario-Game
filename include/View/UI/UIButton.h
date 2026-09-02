#ifndef VIEW_UI_UIBUTTON_H
#define VIEW_UI_UIBUTTON_H

#include "View/UI/UIElement.h"
#include "View/UI/IButtonSkin.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include "View/UI/UITheme.h"

#include <functional>
#include <string>
#include <memory>

namespace view {
namespace ui {

// An interactive button: text label centred on a skin.
// Strategy Pattern: The appearance of the button is delegated to an IButtonSkin.
class UIButton : public UIElement, public IClickable {
public:
    UIButton() = default;

    UIButton(const sf::Font& font, const std::string& labelStr,
             unsigned int charSize, sf::Vector2f position, sf::Vector2f size);

    void setOnClick(std::function<void()> callback) { onClickCallback = std::move(callback); }

    void setLabel(const std::string& text);
    void setColors(sf::Color normal, sf::Color hovered, sf::Color text);
    void setFont(const sf::Font& font, unsigned int cs);

    // Strategy Pattern: Swap skin at runtime
    void setSkin(std::unique_ptr<IButtonSkin> newSkin);

    // Mask mode: skin stays transparent even when disabled
    void setMaskMode(bool enable);

    void setPosition(float x, float y) override;
    void setSize(float w, float h) override;

    void render(sf::RenderTarget& target) override;
    bool handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void onMouseLeave() override;
    void setEnabled(bool e) override;

    void onHover(bool hovered) override;
    void onClick() override;

private:
    std::unique_ptr<IButtonSkin> skin;

    const sf::Font* fontPtr   = nullptr;
    std::string     labelStr;
    unsigned int    charSize  = 8u;

    sf::Color colorText       = theme::ColorText;

    bool isHovered = false;

    std::function<void()> onClickCallback;
};

}  // namespace ui
}  // namespace view

#endif
