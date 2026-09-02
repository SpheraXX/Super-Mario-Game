#include "Controller/CustomMapBrowseState.h"

#include "Controller/AppEngine.h"
#include "Controller/CustomMapHubState.h"
#include "Controller/LoadingState.h"
#include "Controller/PlayState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Editor/EditableMap.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <filesystem>
#include <memory>

namespace controller {

CustomMapBrowseState::CustomMapBrowseState()
    : m_background(view::AssetManager::instance().getTexture("assets/images/bga_mainmenu.png")) {
    m_background.setColor(view::ui::theme::BgaDimMenu);

    const sf::Font& font = view::AssetManager::instance().getUiFont();
    m_backButton = view::ui::UIButton(font, "BACK", view::ui::layout::ButtonFontSize, {0, 0},
                                      {view::ui::layout::MenuButtonWidth, view::ui::layout::MenuButtonHeight});
    m_backButton.setOnClick([this]() {
        manager->replaceState(std::make_unique<CustomMapHubState>());
    });
}

void CustomMapBrowseState::onEnter() {
    buildUI();
    onDisplayModeChanged();
}

void CustomMapBrowseState::buildUI() {
    m_grid.clear();
    m_grid = view::ui::UIContainer(view::ui::UIContainer::Layout::None, 0);

    const auto mapPaths = model::EditableMap::listCustomMaps();
    m_hasMaps = !mapPaths.empty();

    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const int itemsPerRow = 4;
    const float padding = view::ui::layout::MenuButtonGap / 1.5f;
    const float btnW = 70.f;
    const float btnH = 24.f;

    int count = 0;
    for (const auto& path : mapPaths) {
        const int row = count / itemsPerRow;
        const int col = count % itemsPerRow;
        const float x = col * (btnW + padding);
        const float y = row * (btnH + padding);

        const std::string label = std::filesystem::path(path).stem().string();
        auto btn = std::make_unique<view::ui::UIButton>(font, label, view::ui::layout::SmallFontSize,
                                                         sf::Vector2f{x, y}, sf::Vector2f{btnW, btnH});

        std::string mapPath = path;
        std::string levelId = label;
        btn->setOnClick([this, mapPath, levelId]() {
            model::GameManager::instance().setCurrentMapPath(mapPath);
            model::GameManager::instance().setLevelName(levelId);
            model::GameManager::instance().setCustomMapSession(true);

            auto playState = std::make_unique<PlayState>();
            auto loadState = std::make_unique<LoadingState>([]() {}, std::move(playState));
            manager->replaceState(std::move(loadState));
        });

        m_grid.add(std::move(btn));
        ++count;
    }

    const int rows = (count + itemsPerRow - 1) / itemsPerRow;
    const float cols = static_cast<float>(std::min(count, itemsPerRow));
    m_grid.setSize(cols * btnW + std::max(0.f, cols - 1.f) * padding,
                   rows * btnH + std::max(0, rows - 1) * padding);

    const sf::Font& labelFont = view::AssetManager::instance().getUiFont();
    m_emptyLabel = view::ui::UILabel(labelFont, "No custom maps yet - create one first!",
                                     view::ui::layout::ButtonFontSize);
}

void CustomMapBrowseState::onDisplayModeChanged() {
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

    m_grid.relayout();
    const float listW = m_grid.getSize().x;
    const float listH = m_grid.getSize().y;
    m_grid.setPosition((W - listW) / 2.f, (H - listH) / 2.f);

    m_emptyLabel.setSize(W, 20.f);
    m_emptyLabel.setPosition(0.f, H / 2.f - 10.f);

    const float btnW = view::ui::layout::MenuButtonWidth;
    const float btnH = view::ui::layout::MenuButtonHeight;
    const float padding = view::ui::layout::MenuButtonGap;
    m_backButton.setSize(btnW, btnH);
    m_backButton.setPosition(padding, H - btnH - padding);
}

void CustomMapBrowseState::handleEvent(const sf::Event& event) {
    m_grid.handleEvent(event);
    m_backButton.handleEvent(event);

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            manager->replaceState(std::make_unique<CustomMapHubState>());
        }
    }
}

void CustomMapBrowseState::update(float dt) {
    m_grid.update(dt);
    m_backButton.update(dt);
}

void CustomMapBrowseState::render(sf::RenderTarget& target) {
    target.draw(m_background);
    if (m_hasMaps) {
        m_grid.render(target);
    } else {
        m_emptyLabel.render(target);
    }
    m_backButton.render(target);
}

} // namespace controller
