#ifndef CONTROLLER_CUSTOMMAPBROWSESTATE_H
#define CONTROLLER_CUSTOMMAPBROWSESTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIButton.h"
#include "View/UI/UIContainer.h"
#include "View/UI/UILabel.h"
#include <SFML/Graphics/Sprite.hpp>

namespace controller {

// "PLAY CUSTOM MAP": lists the .map files under assets/maps/custom/ and launches
// PlayState directly on whichever one is picked.
class CustomMapBrowseState : public GameState {
public:
    CustomMapBrowseState();

    void onEnter() override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void buildUI();

    sf::Sprite m_background;
    view::ui::UIContainer m_grid;
    view::ui::UIButton m_backButton;
    view::ui::UILabel m_emptyLabel;
    bool m_hasMaps = false;
};

} // namespace controller

#endif // CONTROLLER_CUSTOMMAPBROWSESTATE_H
