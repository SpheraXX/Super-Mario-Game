#ifndef CONTROLLER_WORLDSELECTSTATE_H
#define CONTROLLER_WORLDSELECTSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIButton.h"
#include "View/Effect/LerpAnimator.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <vector>

namespace controller {

class WorldSelectState : public GameState {
public:
    explicit WorldSelectState(int initialFocusIndex = 0);
    ~WorldSelectState() override = default;

    void onEnter() override;
    void onDisplayModeChanged() override;
    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;

private:
    void updateCarousel();
    void updateCardLayout();
    void buildUI();

    int m_focusIndex = 0;
    std::vector<sf::Sprite> m_worldSprites;
    std::vector<std::string> m_worldIds;
    
    // Lerp animator for horizontal slide
    view::effect::LerpAnimator m_slideAnimator{0.f, 0.f, 0.3f, view::effect::Easing::OutCubic};
    float m_targetX = 0.f;

    sf::Sprite m_background;
    
    // Back to main menu
    view::ui::UIButton m_backButton;
};

} // namespace controller

#endif // CONTROLLER_WORLDSELECTSTATE_H
