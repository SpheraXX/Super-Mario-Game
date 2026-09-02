#include "Controller/LevelSelectState.h"
#include "Controller/AppEngine.h"
#include "Controller/WorldSelectState.h"
#include "Controller/LoadingState.h"
#include "Controller/PlayState.h"
#include "Model/Core/WorldManager.h"
#include "Model/Save/SaveManager.h"
#include "View/AssetManager.h"
#include "View/UI/UITheme.h"

namespace {
// SOLID-3 fix: replace magic strings with a type-safe enum
enum class LevelStatus { Available, Passed, Locked };
} // namespace

namespace controller {

LevelSelectState::LevelSelectState(std::string worldId) 
    : m_worldId(worldId),
      m_background(view::AssetManager::instance().getTexture(
          model::WorldManager::instance().getWorld(worldId) && !model::WorldManager::instance().getWorld(worldId)->bgaImage.empty() 
          ? model::WorldManager::instance().getWorld(worldId)->bgaImage 
          : "assets/images/bga_mainmenu.png" // fallback
      )) 
{
    m_background.setColor(view::ui::theme::BgaDimMenu);

    const sf::Font& font = view::AssetManager::instance().getUiFont();
    m_backButton = view::ui::UIButton(font, "BACK TO WORLDS", view::ui::layout::ButtonFontSize, {0, 0}, {view::ui::layout::MenuButtonWidth * 1.5f, view::ui::layout::MenuButtonHeight});
    m_backButton.setOnClick([this]() {
        int focusIndex = 0;
        const auto& worlds = model::WorldManager::instance().getWorlds();
        for (size_t i = 0; i < worlds.size(); ++i) {
            if (worlds[i].id == m_worldId) {
                focusIndex = static_cast<int>(i);
                break;
            }
        }
        manager->replaceState(std::make_unique<WorldSelectState>(focusIndex));
    });
}

void LevelSelectState::onEnter() {
    buildUI();
    onDisplayModeChanged();
}

void LevelSelectState::buildUI() {
    m_grid.clear();
    
    // Grid layout: we can just use a vertical container of horizontal containers, 
    // or since UIContainer is purely vertical or horizontal, we can simulate grid.
    // Let's assume a simple list for now, or a few horizontal containers inside a vertical one.

    const auto* world = model::WorldManager::instance().getWorld(m_worldId);
    if (!world) {
        return;
    }

    // Load save data once for all levels
    model::GameSaveData saveData;
    // BUG-5 fix: use SaveManager::DefaultSavePath instead of hardcoded "save.json"
    model::SaveManager::instance().load(saveData, model::SaveManager::DefaultSavePath);

    m_grid = view::ui::UIContainer(view::ui::UIContainer::Layout::None, 0);
    
    int itemsPerRow = 4;
    float padding = view::ui::layout::MenuButtonGap;
    float btnSize = 80.f;
    
    int count = 0;
    const sf::Font& font = view::AssetManager::instance().getUiFont();

    for (const auto& lvl : world->levels) {
        int row = count / itemsPerRow;
        int col = count % itemsPerRow;
        
        float x = col * (btnSize + padding);
        float y = row * (btnSize + padding);

        auto btn = std::make_unique<view::ui::UIButton>(font, lvl.id, view::ui::layout::ButtonFontSize, sf::Vector2f{x, y}, sf::Vector2f{btnSize, btnSize});
        
        // SOLID-3 fix: use enum instead of magic strings
        LevelStatus status = LevelStatus::Available; // DEBUG: allow all for now
        auto it = saveData.level_progress.find(lvl.id);
        if (it != saveData.level_progress.end()) {
            if (it->second == "pass")        status = LevelStatus::Passed;
            else if (it->second == "lock")   status = LevelStatus::Locked;
            else                             status = LevelStatus::Available;
        }

        if (status == LevelStatus::Locked) {
            btn->setEnabled(false);
        }
        // TODO: Apply different visual skin for Passed vs Available when skin system supports it
        
        std::string mapPath = lvl.mapPath;
        btn->setOnClick([this, mapPath]() {
            // BUG-5 fix: use SaveManager::DefaultSavePath
            model::GameSaveData data;
            model::SaveManager::instance().load(data, model::SaveManager::DefaultSavePath);
            data.level.mapPath = mapPath;
            model::SaveManager::instance().save(data, model::SaveManager::DefaultSavePath);

            auto playState = std::make_unique<PlayState>();
            auto loadState = std::make_unique<LoadingState>(
                []() {}, // No heavy loading callback for now
                std::move(playState)
            );
            manager->replaceState(std::move(loadState));
        });

        m_grid.add(std::move(btn));
        count++;
    }
    
    // Manually set m_grid size
    int rows = (count + itemsPerRow - 1) / itemsPerRow;
    float cols = static_cast<float>(std::min(count, itemsPerRow));
    m_grid.setSize(cols * btnSize + (cols - 1) * padding, rows * btnSize + (rows - 1) * padding);
}

void LevelSelectState::onDisplayModeChanged() {
    float W = static_cast<float>(AppEngine::screenWidth());
    float H = static_cast<float>(AppEngine::ScreenHeight);
    
    const sf::Texture &tex = m_background.getTexture();
    float scaleX = W / static_cast<float>(tex.getSize().x);
    float scaleY = H / static_cast<float>(tex.getSize().y);
    float scale = std::max(scaleX, scaleY) * view::ui::layout::BgaScaleMultiplier;
    m_background.setScale({scale, scale});
    m_background.setOrigin({static_cast<float>(tex.getSize().x) / 2.f,
                            static_cast<float>(tex.getSize().y) / 2.f});
    m_background.setPosition({W / 2.f, H / 2.f});
    
    m_grid.relayout();
    float listW = m_grid.getSize().x;
    float listH = m_grid.getSize().y;
    m_grid.setPosition((W - listW) / 2.f, (H - listH) / 2.f);
    
    float btnW = view::ui::layout::MenuButtonWidth * 1.5f;
    float btnH = view::ui::layout::MenuButtonHeight;
    float padding = view::ui::layout::MenuButtonGap;
    
    m_backButton.setSize(btnW, btnH);
    m_backButton.setPosition(padding, H - btnH - padding);
}

void LevelSelectState::handleEvent(const sf::Event& event) {
    m_grid.handleEvent(event);
    m_backButton.handleEvent(event);
    
    // BUG-6 fix: use single getIf<> instead of is<> + getIf<>
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            int focusIndex = 0;
            const auto& worlds = model::WorldManager::instance().getWorlds();
            for (size_t i = 0; i < worlds.size(); ++i) {
                if (worlds[i].id == m_worldId) {
                    focusIndex = static_cast<int>(i);
                    break;
                }
            }
            manager->replaceState(std::make_unique<WorldSelectState>(focusIndex));
        }
    }
}

void LevelSelectState::update(float dt) {
    m_grid.update(dt);
    m_backButton.update(dt);
}

void LevelSelectState::render(sf::RenderTarget& target) {
    target.draw(m_background);
    m_grid.render(target);
    m_backButton.render(target);
}

} // namespace controller
