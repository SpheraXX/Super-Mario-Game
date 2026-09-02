#include "Controller/CustomMapHubState.h"

#include "Controller/AppEngine.h"
#include "Controller/CustomMapBrowseState.h"
#include "Controller/CustomMapWorldPickState.h"
#include "Controller/MainMenuState.h"
#include "Controller/StateManager.h"
#include "View/AssetManager.h"
#include "View/UI/UIButton.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>

namespace controller {

CustomMapHubState::CustomMapHubState()
    : m_background(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu.png")) {
    m_background.setColor(view::ui::theme::BgaDimMenu);
}

void CustomMapHubState::onEnter() {
    buildUI();
    onDisplayModeChanged();
}

namespace {
// Wider/taller than the standard menu button: "CREATE NEW MAP"/"PLAY CUSTOM MAP" don't
// fit the shared MenuButtonWidth/Height without clipping.
constexpr float ButtonWidth = view::ui::layout::MenuButtonWidth * 1.7f;
constexpr float ButtonHeight = view::ui::layout::MenuButtonHeight + 6.f;
}

void CustomMapHubState::buildUI() {
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const float W = static_cast<float>(AppEngine::screenWidth());

    m_buttons = view::ui::UIContainer(view::ui::UIContainer::Layout::Vertical,
                                      view::ui::layout::MenuButtonGap);
    const float listX = (W - ButtonWidth) / 2.f;
    m_buttons.setPosition(listX, 0.f);
    m_buttons.setSize(ButtonWidth, 0.f);

    auto makeBtn = [&](const std::string& label, std::function<void()> cmd) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, label, view::ui::layout::ButtonFontSize, sf::Vector2f{0.f, 0.f},
            sf::Vector2f{ButtonWidth, ButtonHeight});
        btn->setOnClick(std::move(cmd));
        m_buttons.add(std::move(btn));
    };

    makeBtn("CREATE NEW MAP", [this]() {
        manager->pushState(std::make_unique<CustomMapWorldPickState>());
    });

    makeBtn("PLAY CUSTOM MAP", [this]() {
        manager->pushState(std::make_unique<CustomMapBrowseState>());
    });

    makeBtn("BACK", [this]() {
        manager->replaceState(std::make_unique<MainMenuState>());
    });
}

void CustomMapHubState::onDisplayModeChanged() {
    const float W = static_cast<float>(AppEngine::screenWidth());
    const float H = static_cast<float>(AppEngine::ScreenHeight);

    const sf::Texture& tex = m_background.getTexture();
    float scaleX = W / static_cast<float>(tex.getSize().x);
    float scaleY = H / static_cast<float>(tex.getSize().y);
    float scale = std::max(scaleX, scaleY) * view::ui::layout::BgaScaleMultiplier;
    m_background.setScale({scale, scale});
    m_background.setOrigin({static_cast<float>(tex.getSize().x) / 2.f,
                            static_cast<float>(tex.getSize().y) / 2.f});
    m_background.setPosition({W / 2.f, H / 2.f});

    const float listX = (W - ButtonWidth) / 2.f;
    m_buttons.setPosition(listX, H * 0.4f);
    m_buttons.relayout();
}

void CustomMapHubState::handleEvent(const sf::Event& event) {
    m_buttons.handleEvent(event);

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            manager->replaceState(std::make_unique<MainMenuState>());
        }
    }
}

void CustomMapHubState::update(float deltaTime) {
    m_buttons.update(deltaTime);
}

void CustomMapHubState::render(sf::RenderTarget& target) {
    target.draw(m_background);
    m_buttons.render(target);
}

} // namespace controller
