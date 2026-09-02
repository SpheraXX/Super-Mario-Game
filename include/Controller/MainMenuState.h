#ifndef CONTROLLER_MAINMENUSTATE_H
#define CONTROLLER_MAINMENUSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIContainer.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIButton.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include "View/Effect/LerpAnimator.h"
#include "View/Effect/MetalShineEffect.h"

#include <memory>

namespace controller {

// The main menu screen: title + a vertical list of buttons built from the
// UI framework (UIContainer + UIButton with Command-Pattern callbacks).
//
// DIP compliance: this state does NOT include GameManager, PlayState, or any
// gameplay header directly in this header. Dependencies on gameplay are injected
// via lambda callbacks assigned to each UIButton (see .cpp).
class MainMenuState : public GameState {
public:
    MainMenuState();
    void onEnter() override;
    void onExit()  override {}
    void onResume() override;
    void onDisplayModeChanged() override;

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    // Builds the button list and injects Command callbacks.
    void buildUI();

    // Background image
    sf::Sprite bgaSprite;

    // Title image (replaces text).
    sf::Sprite titleSprite;
    
    // Animation properties
    view::effect::LerpAnimator m_menuSlideIn;
    view::effect::MetalShineEffect m_titleShine;
    float baseTitleY = 0.0f;

    // The button stack (Vertical UIContainer).
    view::ui::UIContainer menuList;

    bool fontLoaded = false;
};

}  // namespace controller

#endif
