#ifndef CONTROLLER_PROFILEINPUTPOPUPSTATE_H
#define CONTROLLER_PROFILEINPUTPOPUPSTATE_H

#include "Controller/GameState.h"
#include "View/UI/UIButton.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIContainer.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <functional>
#include <string>

namespace controller {

class ProfileInputPopupState : public GameState {
public:
    ProfileInputPopupState(const std::string& initialName, std::function<void(const std::string&)> onConfirm);

    void onEnter() override;
    void update(float dt) override;
    void render(sf::RenderTarget& target) override;
    void handleEvent(const sf::Event& event) override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }
    
    bool isTransparent() const override { return true; }

private:
    void relayout(float screenW, float screenH);
    void updateInputLabel();
    
    std::string currentText;
    std::function<void(const std::string&)> onConfirmCallback;
    
    sf::RectangleShape overlay;
    sf::RectangleShape inputBox;
    view::ui::UILabel titleLabel;
    view::ui::UILabel inputLabel;
    view::ui::UIContainer buttonContainer;

    float blinkTimer = 0.f;
    bool showCursor = true;
};

} // namespace controller

#endif // CONTROLLER_PROFILEINPUTPOPUPSTATE_H
