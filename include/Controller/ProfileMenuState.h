#ifndef CONTROLLER_PROFILEMENUSTATE_H
#define CONTROLLER_PROFILEMENUSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIContainer.h"
#include "View/Effect/LerpAnimator.h"
#include <SFML/Graphics/Sprite.hpp>

namespace controller {

class ProfileMenuState : public GameState {
public:
    ProfileMenuState();
    ~ProfileMenuState() override = default;

    void onEnter() override;
    void onDisplayModeChanged() override;
    void onResume() override;
    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void buildUI();

    view::ui::UIContainer menuList;
    view::effect::LerpAnimator m_menuSlideIn{0.f, 0.f, 0.6f, view::effect::Easing::OutQuad};
    
    sf::Sprite bgaSprite;
    sf::Sprite titleSprite;
    float baseTitleY = 0.f;
};

} // namespace controller

#endif // CONTROLLER_PROFILEMENUSTATE_H
