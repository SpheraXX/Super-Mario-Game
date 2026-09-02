#ifndef CONTROLLER_CUSTOMMAPWORLDPICKSTATE_H
#define CONTROLLER_CUSTOMMAPWORLDPICKSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIContainer.h"
#include <SFML/Graphics/Sprite.hpp>

namespace controller {

// Step one of "CREATE NEW MAP": pick which landscape (WorldType) the new map uses
// before the grid editor allocates its canvas.
class CustomMapWorldPickState : public GameState {
public:
    CustomMapWorldPickState();

    void onEnter() override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    void buildUI();

    sf::Sprite m_background;
    view::ui::UIContainer m_buttons;
};

} // namespace controller

#endif // CONTROLLER_CUSTOMMAPWORLDPICKSTATE_H
