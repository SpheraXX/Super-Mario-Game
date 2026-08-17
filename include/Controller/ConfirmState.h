#ifndef CONTROLLER_CONFIRMSTATE_H
#define CONTROLLER_CONFIRMSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIButton.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIContainer.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <functional>
#include <string>

namespace controller {

class ConfirmState : public GameState {
public:
    ConfirmState(const std::string& message, std::function<void()> onYes, std::function<void()> onNo);

    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    void handleEvent(const sf::Event& event) override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }
    
    bool isTransparent() const override { return true; }

private:
    void relayout(float screenW, float screenH);
    
    // Layout constants
    static constexpr float kBtnContainerW = 130.f; // YES(60) + gap(10) + NO(60)
    static constexpr float kLabelOffsetY  = -35.f;
    static constexpr float kBtnOffsetY    = +15.f;

    std::string message;
    std::function<void()> onYesCallback;
    std::function<void()> onNoCallback;

    sf::RectangleShape overlay;
    
    view::ui::UILabel titleLabel;
    view::ui::UIContainer buttonContainer;
};

} // namespace controller

#endif
