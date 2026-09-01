#ifndef CONTROLLER_LOADINGSTATE_H
#define CONTROLLER_LOADINGSTATE_H

#include "Controller/GameState.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>
#include <functional>
#include <string>

namespace controller {

class LoadingState : public GameState {
public:
    LoadingState(std::function<void()> workCallback, std::unique_ptr<GameState> nextState);

    void onEnter() override;
    void onExit() override {}
    void onResume() override {}
    void onDisplayModeChanged() override;

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    std::function<void()> m_workCallback;
    std::unique_ptr<GameState> m_nextState;
    
    sf::RectangleShape m_background;
    sf::Sprite m_spinnerSprite;
    float m_rotation = 0.f;
    bool m_firstFrameDone = false;
};

} // namespace controller

#endif
