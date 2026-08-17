#include "Controller/ConfirmState.h"
#include "Controller/AppEngine.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"
#include <SFML/Graphics/RenderTarget.hpp>

namespace controller {

ConfirmState::ConfirmState(const std::string& msg, std::function<void()> onYes, std::function<void()> onNo)
    : message(msg), onYesCallback(onYes), onNoCallback(onNo), buttonContainer(view::ui::UIContainer::Layout::None) {
    
    // Dim the background
    overlay.setFillColor(view::ui::theme::ColorOverlay);
    
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    
    titleLabel = view::ui::UILabel(font, message, 10);
    titleLabel.setLineSpacing(1.5f);
    
    auto btnYes = std::make_unique<view::ui::UIButton>(font, "YES", 8, sf::Vector2f(0.f, 0.f), sf::Vector2f(60.f, 20.f));
    btnYes->setOnClick([this]() {
        if (onYesCallback) onYesCallback();
    });
    
    auto btnNo = std::make_unique<view::ui::UIButton>(font, "NO", 8, sf::Vector2f(70.f, 0.f), sf::Vector2f(60.f, 20.f));
    btnNo->setOnClick([this]() {
        if (onNoCallback) onNoCallback();
    });
    
    buttonContainer.add(std::move(btnYes));
    buttonContainer.add(std::move(btnNo));

    onDisplayModeChanged();
}

void ConfirmState::onDisplayModeChanged() {
    float screenW = static_cast<float>(AppEngine::screenWidth());
    float screenH = static_cast<float>(AppEngine::ScreenHeight);
    relayout(screenW, screenH);
}

void ConfirmState::relayout(float screenW, float screenH) {
    overlay.setSize({screenW, screenH});
    
    // Force UILabel to span the full width, so its internal centering handles the text.
    titleLabel.setSize(screenW, 20.f);
    titleLabel.setPosition(0.f, (screenH / 2.f) + kLabelOffsetY);
    
    // Center button container
    buttonContainer.setPosition((screenW - kBtnContainerW) / 2.f, (screenH / 2.f) + kBtnOffsetY);
}

void ConfirmState::update(float dt) {
    buttonContainer.update(dt);
}

void ConfirmState::handleEvent(const sf::Event& event) {
    buttonContainer.handleEvent(event);
}

void ConfirmState::render(sf::RenderTarget& target) {
    target.draw(overlay);
    titleLabel.render(target);
    buttonContainer.render(target);
}

} // namespace controller
