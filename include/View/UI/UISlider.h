#ifndef VIEW_UI_UISLIDER_H
#define VIEW_UI_UISLIDER_H

#include "View/UI/UIElement.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <functional>
#include <string>

namespace view {
namespace ui {

class UISlider : public UIElement {
public:
    UISlider() = default;

    UISlider(const sf::Font& font, const std::string& label,
             int min, int max, int step, int initial,
             sf::Vector2f position, sf::Vector2f size);

    void setOnChange(std::function<void(int)> cb) { onChange = std::move(cb); }
    void setValue(int v);
    int  getValue() const { return value; }
    void setColors(sf::Color track, sf::Color fill, sf::Color knob, sf::Color text);

    void setPosition(float x, float y) override;
    void setSize(float w, float h) override;

    void render(sf::RenderTarget& target) override;
    bool handleEvent(const sf::Event& event) override;

private:
    void clampAndSnap();
    float knobX() const;
    void updateLayout();

    const sf::Font* fontPtr  = nullptr;
    std::string     labelStr;

    int minVal  = 0;
    int maxVal  = 100;
    int stepVal = 10;
    int value   = 50;

    sf::RectangleShape track;   
    sf::RectangleShape fill;    
    sf::RectangleShape knob;    

    sf::Color colorTrack = sf::Color(50, 50, 70);
    sf::Color colorFill  = sf::Color(80, 120, 200);
    sf::Color colorKnob  = sf::Color(180, 210, 255);
    sf::Color colorText  = sf::Color::White;

    bool dragging = false;
    static constexpr float KnobW = 6.f;
    static constexpr unsigned int LabelSize = 6u;

    std::function<void(int)> onChange;
};

}  // namespace ui
}  // namespace view

#endif
