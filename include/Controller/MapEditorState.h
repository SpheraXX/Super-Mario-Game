#ifndef CONTROLLER_MAPEDITORSTATE_H
#define CONTROLLER_MAPEDITORSTATE_H

#include "Controller/GameState.h"
#include "Model/Editor/EditableMap.h"
#include "Model/World/WorldType.h"
#include "View/Map/TileMapRenderer.h"
#include "View/UI/UIButton.h"
#include "View/UI/UICycleButton.h"
#include "View/UI/UILabel.h"
#include "View/UI/UIScrollView.h"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <unordered_map>
#include <vector>

namespace controller {

// The grid editor screen: paints/erases entities on a fixed-size single-area map and
// saves it to assets/maps/custom/. Reached via CustomMapWorldPickState, which supplies
// the landscape chosen for the new map.
class MapEditorState : public GameState {
public:
    explicit MapEditorState(model::WorldType worldType);

    void onEnter() override;
    void onDisplayModeChanged() override;
    void onResume() override { onDisplayModeChanged(); }

    void handleEvent(const sf::Event& event) override;
    void update(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

private:
    struct PaletteEntry {
        char symbol;
        view::ui::UIButton* button;
    };

    // The real sprite (sheet + source rect) each palette symbol draws as, used for the
    // faded placement preview at the hovered cell. Built once in onEnter() from the same
    // atlas rects the actual game renderers use (TileMapRenderer, EnemyAtlas, MiscAtlas,
    // CoinBlockRenderer, LevelGoalRenderer, PlayerRenderer).
    struct IconInfo {
        const sf::Texture* texture = nullptr;
        sf::IntRect rect;
    };

    void buildPalette();
    void buildIcons();
    void layoutUI();
    void selectPaletteEntry(char symbol, view::ui::UIButton* button);
    // Resolves a window-space point to a grid cell, honoring the current scroll offset
    // and excluding the toolbar/palette screen-space strips. Returns false (leaving
    // outRow/outColumn untouched) when the point isn't over the grid.
    bool resolveCell(sf::Vector2i windowPos, std::size_t& outRow, std::size_t& outColumn) const;
    void paintAt(sf::Vector2i windowPos);
    void clampScroll();
    void renderEntityMarkers(sf::RenderTarget& target) const;
    void renderGhostPreview(sf::RenderTarget& target) const;
    void trySave();

    model::EditableMap m_map;
    view::TileMapRenderer m_renderer;

    float m_scrollX = 0.f;
    bool m_mouseDown = false;

    char m_selectedSymbol = 'G';
    view::ui::UIButton* m_selectedButton = nullptr;
    std::vector<PaletteEntry> m_paletteEntries;
    std::unordered_map<char, IconInfo> m_icons;

    bool m_hoverValid = false;
    std::size_t m_hoverRow = 0;
    std::size_t m_hoverColumn = 0;

    view::ui::UIScrollView m_palette;
    view::ui::UICycleButton m_modeToggle;
    view::ui::UIButton m_saveButton;
    view::ui::UIButton m_backButton;
    view::ui::UILabel m_statusLabel;
};

} // namespace controller

#endif // CONTROLLER_MAPEDITORSTATE_H
