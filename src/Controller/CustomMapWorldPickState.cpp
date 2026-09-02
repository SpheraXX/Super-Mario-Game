#include "Controller/CustomMapWorldPickState.h"

#include "Controller/AppEngine.h"
#include "Controller/CustomMapHubState.h"
#include "Controller/MapEditorState.h"
#include "Controller/StateManager.h"
#include "Model/World/WorldType.h"
#include "View/AssetManager.h"
#include "View/UI/UIButton.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <functional>
#include <memory>
#include <string>

namespace controller {

namespace {
// Wider/taller than the standard menu button: "UNDERGROUND"/"UNDERWATER" are longer than
// the single short words MenuButtonWidth/Height were originally sized for.
constexpr float ButtonWidth = view::ui::layout::MenuButtonWidth * 1.35f;
constexpr float ButtonHeight = view::ui::layout::MenuButtonHeight + 6.f;
}

CustomMapWorldPickState::CustomMapWorldPickState()
    : m_background(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu.png")) {
    m_background.setColor(view::ui::theme::BgaDimMenu);
}

void CustomMapWorldPickState::onEnter() {
    buildUI();
    onDisplayModeChanged();
}

void CustomMapWorldPickState::buildUI() {
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const float W = static_cast<float>(AppEngine::screenWidth());

    m_buttons = view::ui::UIContainer(view::ui::UIContainer::Layout::Vertical,
                                      view::ui::layout::MenuButtonGap);
    const float listX = (W - ButtonWidth) / 2.f;
    m_buttons.setPosition(listX, 0.f);
    m_buttons.setSize(ButtonWidth, 0.f);

    auto makeBtn = [&](const std::string& label, model::WorldType world) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, label, view::ui::layout::ButtonFontSize, sf::Vector2f{0.f, 0.f},
            sf::Vector2f{ButtonWidth, ButtonHeight});
        btn->setOnClick([this, world]() {
            manager->replaceState(std::make_unique<MapEditorState>(world));
        });
        m_buttons.add(std::move(btn));
    };

    makeBtn("OVERWORLD", model::WorldType::Overworld);
    makeBtn("UNDERGROUND", model::WorldType::Underground);
    makeBtn("UNDERWATER", model::WorldType::Underwater);
    makeBtn("CASTLE", model::WorldType::Castle);

    auto backBtn = std::make_unique<view::ui::UIButton>(
        font, "BACK", view::ui::layout::ButtonFontSize, sf::Vector2f{0.f, 0.f},
        sf::Vector2f{ButtonWidth, ButtonHeight});
    backBtn->setOnClick([this]() {
        manager->replaceState(std::make_unique<CustomMapHubState>());
    });
    m_buttons.add(std::move(backBtn));
}

void CustomMapWorldPickState::onDisplayModeChanged() {
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
    m_buttons.setPosition(listX, H * 0.3f);
    m_buttons.relayout();
}

void CustomMapWorldPickState::handleEvent(const sf::Event& event) {
    m_buttons.handleEvent(event);

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            manager->replaceState(std::make_unique<CustomMapHubState>());
        }
    }
}

void CustomMapWorldPickState::update(float deltaTime) {
    m_buttons.update(deltaTime);
}

void CustomMapWorldPickState::render(sf::RenderTarget& target) {
    target.draw(m_background);
    m_buttons.render(target);
}

} // namespace controller
