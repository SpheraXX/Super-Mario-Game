#include "Controller/MapEditorState.h"

#include "Controller/AppEngine.h"
#include "Controller/CustomMapHubState.h"
#include "Controller/ProfileInputPopupState.h"
#include "Controller/StateManager.h"
#include "Controller/WarningPopupState.h"
#include "Model/World/WorldSet.h"
#include "Model/World/WorldTheme.h"
#include "View/AssetManager.h"
#include "View/Block/BlockAtlas.h"
#include "View/UI/UITheme.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>

namespace controller {

namespace {
struct PaletteDef {
    char symbol;
    const char* label;
};

// Every placeable entity/tile the game has, minus the castle backdrop pieces (not wanted
// in the editor), the moving Slider platform (needs an auto-generated header line, out
// of scope for v1 — see the map editor plan) and warp pipes (never emitted: painting
// P/Q/p/q here is always plain solid terrain, since no '; pipe=' header is ever written
// for a custom map).
constexpr PaletteDef kPalette[] = {
    {'G', "GROUND"},   {'s', "STAIR"},     {'O', "CLOUD"},      {'T', "TREE"},
    {'w', "BUSH"},     {'m', "HILL"},      {'k', "KELP"},       {'c', "CHAIN"},
    {'v', "LAVA TOP"}, {'x', "LAVA"},
    {'P', "PIPE TL"},  {'Q', "PIPE TR"},   {'p', "PIPE BL"},    {'q', "PIPE BR"},
    {'C', "COIN BLK"}, {'#', "BRICK"},     {'$', "COIN"},       {'F', "H-PIPE"},
    {'r', "FIREBAR"},  {'b', "LAVA BUB"},  {'X', "AXE"},        {'R', "TOAD"},
    {'E', "GOAL"},     {'M', "PLAYER START"},
    {'0', "GOOMBA"},   {'1', "KOOPA"},     {'2', "PARATROOPA"}, {'3', "HAMMERBRO"},
    {'4', "LAKITU"},   {'5', "SPINY"},     {'6', "CHEEPCHEEP"}, {'7', "BOWSER"},
    {'8', "PIRANHA"},
};

// Symbols LevelScene spawns as entities (rather than drawing as terrain via
// TileMapRenderer): the editor draws the same real sprite the entity would spawn as,
// looked up in m_icons, since the live preview never actually spawns real entities.
const std::string kEntitySymbols = "C#$FrbXREM0123456789";

constexpr float PaletteWidth = 110.f;
constexpr float PaletteButtonHeight = 18.f;
constexpr float PaletteButtonGap = 3.f;
constexpr float ToolbarHeight = 22.f;
constexpr float ToolbarButtonWidth = 84.f;
constexpr float ScrollKeySpeed = 220.f;
constexpr float ScrollWheelStep = 32.f;
} // namespace

MapEditorState::MapEditorState(model::WorldType worldType)
    : m_map(worldType), m_renderer("assets/blocks.png", worldType) {
}

void MapEditorState::onEnter() {
    const sf::Font& font = view::AssetManager::instance().getUiFont();

    m_modeToggle = view::ui::UICycleButton(font, "MODE", {"PUT", "ERASE"}, 0,
                                           sf::Vector2f{0.f, 0.f},
                                           sf::Vector2f{view::ui::layout::SmallButtonWidth, ToolbarHeight});

    m_saveButton = view::ui::UIButton(font, "SAVE", view::ui::layout::ButtonFontSize,
                                      sf::Vector2f{0.f, 0.f},
                                      sf::Vector2f{view::ui::layout::SmallButtonWidth, ToolbarHeight});
    m_saveButton.setOnClick([this]() { trySave(); });

    m_backButton = view::ui::UIButton(font, "BACK", view::ui::layout::ButtonFontSize,
                                      sf::Vector2f{0.f, 0.f},
                                      sf::Vector2f{view::ui::layout::SmallButtonWidth, ToolbarHeight});
    m_backButton.setOnClick([this]() {
        manager->replaceState(std::make_unique<CustomMapHubState>());
    });

    m_statusLabel = view::ui::UILabel(font, "", view::ui::layout::SmallFontSize, sf::Color::Yellow);
    m_statusLabel.setCentered(false);

    buildPalette();
    buildIcons();
    layoutUI();
}

void MapEditorState::buildIcons() {
    view::AssetManager& assets = view::AssetManager::instance();
    const model::WorldType world = m_map.getWorldType();

    // Same backdrop colors TileMapRenderer/EnemyAtlas/MiscAtlas key out of their sheets
    // (View/Map/TileMapRenderer.cpp, View/Enemy/EnemyAtlas.h, View/Base/MiscAtlas.h) --
    // duplicated here since those constants/helpers are private to their own renderers.
    // AssetManager::getTexture caches one masked variant per (path, colorKey) pair, so
    // requesting the same sheet with different keys below is cheap after the first call.
    const sf::Color sceneryKey(148, 148, 255);
    const sf::Color kelpKey(66, 66, 255);
    const sf::Color blackKey(0, 0, 0);
    const sf::Color enemyKey(146, 144, 255);

    const std::string marioAsset = "assets/super_mario_asset.png";
    const sf::Texture& marioPlain   = assets.getTexture(marioAsset);
    const sf::Texture& marioScenery = assets.getTexture(marioAsset, sceneryKey);
    const sf::Texture& marioKelp    = assets.getTexture(marioAsset, kelpKey);
    const sf::Texture& marioBlack   = assets.getTexture(marioAsset, blackKey);
    const sf::Texture& enemySheet   = assets.getTexture("assets/enemies-8.png", enemyKey);
    const sf::Texture& miscSheet    = assets.getTexture("assets/misc.png", blackKey);
    const sf::Texture& blocksSheet  = assets.getTexture("assets/blocks.png");
    const sf::Texture& marioLuigi   = assets.getTexture("assets/mario-luigi.png");

    const sf::Vector2i groundPx = view::atlas::groundOrigin(world);
    const sf::Vector2i brickPx = view::atlas::brickOrigin(world);

    // Mirrors TileMapRenderer::atlasFor's per-landscape scenery origin (private to that
    // class), needed only for the two composite tiles (bush/hill) that vary by landscape.
    int sceneryX = 0;
    int sceneryY = 213;
    switch (world) {
        case model::WorldType::Underground: sceneryX = 164; sceneryY = 213; break;
        case model::WorldType::Underwater:  sceneryX = 164; sceneryY = 297; break;
        case model::WorldType::Castle:      sceneryX = 0;   sceneryY = 297; break;
        case model::WorldType::Overworld:
        default:                            sceneryX = 0;   sceneryY = 213; break;
    }
    constexpr int Pitch = 17;

    m_icons.clear();
    auto put = [this](char symbol, const sf::Texture& tex, sf::IntRect rect) {
        m_icons[symbol] = IconInfo{&tex, rect};
    };

    // Terrain that needs no color key (solid artwork, no backdrop pixels).
    put('G', marioPlain, {{groundPx.x, groundPx.y}, {16, 16}});
    put('s', marioPlain, {{groundPx.x, groundPx.y + Pitch}, {16, 16}});
    put('#', marioPlain, {{brickPx.x, brickPx.y}, {16, 16}});
    put('r', marioPlain, {{580, 476}, {16, 16}});
    put('x', marioPlain, {{616, 744}, {16, 16}});

    // Scenery/terrain keyed on the shared blue backdrop.
    put('O', marioScenery, {{344, 632}, {48, 32}});
    put('T', marioScenery, {{264, 656}, {16, 32}});
    put('w', marioScenery, {{sceneryX + Pitch, sceneryY}, {16, 16}});
    put('m', marioScenery, {{sceneryX + 2 * Pitch, sceneryY + 2 * Pitch}, {16, 16}});
    put('A', marioScenery, {{40, 696}, {48, 32}});
    put('H', marioScenery, {{24, 728}, {80, 48}});
    put('P', marioScenery, {{119, 196}, {16, 16}});
    put('Q', marioScenery, {{136, 196}, {16, 16}});
    put('p', marioScenery, {{119, 213}, {16, 16}});
    put('q', marioScenery, {{136, 213}, {16, 16}});
    put('F', marioScenery, {{192, 656}, {64, 32}});
    put('v', marioScenery, {{616, 728}, {16, 16}});
    put('$', marioScenery, {{298, 95}, {16, 16}});

    put('k', marioKelp, {{sceneryX + 3 * Pitch, sceneryY}, {16, 16}});

    put('c', marioBlack, {{548, 476}, {16, 16}});
    put('X', marioBlack, {{580, 460}, {16, 16}});

    put('0', enemySheet, {{0, 16}, {16, 16}});
    put('1', enemySheet, {{0, 113}, {16, 23}});
    put('2', enemySheet, {{54, 113}, {16, 23}});
    put('3', enemySheet, {{18, 183}, {16, 23}});
    put('4', enemySheet, {{54, 138}, {16, 23}});
    put('5', enemySheet, {{72, 352}, {16, 16}});
    put('7', enemySheet, {{102, 208}, {32, 32}});
    put('8', enemySheet, {{0, 139}, {16, 23}});

    put('6', miscSheet, {{240, 184}, {16, 16}});
    put('b', miscSheet, {{0, 154}, {16, 16}});
    put('R', miscSheet, {{244, 272}, {16, 24}});

    put('C', blocksSheet, {{80, 112}, {16, 16}});
    put('E', blocksSheet, {{224, 160}, {16, 16}});

    put('M', marioLuigi, {{176, 32}, {16, 16}});
}

void MapEditorState::buildPalette() {
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    m_palette = view::ui::UIScrollView();
    m_paletteEntries.clear();

    float cursorY = 0.f;
    for (const auto& def : kPalette) {
        auto btn = std::make_unique<view::ui::UIButton>(
            font, def.label, view::ui::layout::SmallFontSize,
            sf::Vector2f{0.f, cursorY}, sf::Vector2f{PaletteWidth - 6.f, PaletteButtonHeight});
        view::ui::UIButton* added = m_palette.add(std::move(btn));
        const char symbol = def.symbol;
        added->setOnClick([this, symbol, added]() { selectPaletteEntry(symbol, added); });
        m_paletteEntries.push_back({symbol, added});
        cursorY += PaletteButtonHeight + PaletteButtonGap;
    }
    m_palette.setContentHeight(cursorY);
    m_palette.setContentWidth(PaletteWidth);

    if (!m_paletteEntries.empty()) {
        selectPaletteEntry(m_paletteEntries.front().symbol, m_paletteEntries.front().button);
    }
}

void MapEditorState::layoutUI() {
    const float screenW = static_cast<float>(AppEngine::screenWidth());
    const float screenH = static_cast<float>(AppEngine::ScreenHeight);

    m_palette.setPosition(screenW - PaletteWidth, 0.f);
    m_palette.setBounds(sf::FloatRect({screenW - PaletteWidth, 0.f}, {PaletteWidth, screenH - ToolbarHeight}));

    m_modeToggle.setPosition(2.f, screenH - ToolbarHeight);
    m_saveButton.setPosition(view::ui::layout::SmallButtonWidth + 4.f, screenH - ToolbarHeight);
    m_backButton.setPosition(2.f * view::ui::layout::SmallButtonWidth + 6.f, screenH - ToolbarHeight);

    m_statusLabel.setPosition(3.f * view::ui::layout::SmallButtonWidth + 10.f, screenH - ToolbarHeight + 3.f);
    m_statusLabel.setSize(screenW - PaletteWidth - (3.f * view::ui::layout::SmallButtonWidth + 10.f), ToolbarHeight);

    clampScroll();
}

void MapEditorState::onDisplayModeChanged() {
    layoutUI();
}

void MapEditorState::selectPaletteEntry(char symbol, view::ui::UIButton* button) {
    if (m_selectedButton) {
        m_selectedButton->setColors(view::ui::theme::ColorNormal, view::ui::theme::ColorHovered,
                                    view::ui::theme::ColorText);
    }
    m_selectedSymbol = symbol;
    m_selectedButton = button;
    if (m_selectedButton) {
        m_selectedButton->setColors(view::ui::theme::ColorSuccessNormal, view::ui::theme::ColorSuccessHovered,
                                    view::ui::theme::ColorText);
    }
}

void MapEditorState::clampScroll() {
    const float mapWidth = static_cast<float>(m_map.columns() * model::TileMap::TileWidth);
    const float screenW = static_cast<float>(AppEngine::screenWidth());
    const float maxScroll = std::max(0.f, mapWidth - screenW);
    m_scrollX = std::clamp(m_scrollX, 0.f, maxScroll);
}

bool MapEditorState::resolveCell(sf::Vector2i windowPos, std::size_t& outRow, std::size_t& outColumn) const {
    const sf::Vector2f logical = AppEngine::windowToLogical(windowPos);
    const float screenH = static_cast<float>(AppEngine::ScreenHeight);
    const float screenW = static_cast<float>(AppEngine::screenWidth());

    // Exclude the toolbar (bottom strip) and palette (right strip) screen-space regions:
    // both sit on top of the grid, so a point over either is never a grid cell.
    if (logical.y < 0.f || logical.y >= screenH - ToolbarHeight) {
        return false;
    }
    if (logical.x >= screenW - PaletteWidth) {
        return false;
    }

    const float worldX = logical.x + m_scrollX;
    if (worldX < 0.f) {
        return false;
    }

    const std::size_t column = static_cast<std::size_t>(worldX / model::TileMap::TileWidth);
    const std::size_t rowFromTop = static_cast<std::size_t>(logical.y / model::TileMap::TileHeight);
    if (column >= m_map.columns() || rowFromTop >= model::TileMap::Rows) {
        return false;
    }

    outColumn = column;
    outRow = model::TileMap::Rows - 1 - rowFromTop;
    return true;
}

void MapEditorState::paintAt(sf::Vector2i windowPos) {
    std::size_t row = 0;
    std::size_t column = 0;
    if (!resolveCell(windowPos, row, column)) {
        return;
    }

    if (m_modeToggle.getIndex() == 1) {
        m_map.setTile(row, column, '.');
    } else if (m_selectedSymbol == 'M') {
        m_map.placePlayerSpawn(row, column);
    } else {
        m_map.setTile(row, column, m_selectedSymbol);
    }
}

void MapEditorState::trySave() {
    const auto validation = m_map.validate();
    if (!validation.ok) {
        manager->pushState(std::make_unique<WarningPopupState>(
            validation.message, WarningPopupState::Type::OkOnly,
            [this]() { manager->popState(); }));
        return;
    }

    manager->pushState(std::make_unique<ProfileInputPopupState>(
        "", [this](const std::string& name) {
            if (name.empty()) {
                return;
            }
            m_map.setLevelName(name);
            if (m_map.saveToFile()) {
                m_statusLabel.setText("SAVED: " + name);
            } else {
                m_statusLabel.setText("SAVE FAILED");
            }
        },
        "ENTER MAP NAME", 20));
}

void MapEditorState::handleEvent(const sf::Event& event) {
    if (m_palette.handleEvent(event)) return;
    if (m_modeToggle.handleEvent(event)) return;
    if (m_saveButton.handleEvent(event)) return;
    if (m_backButton.handleEvent(event)) return;

    if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (pressed->button == sf::Mouse::Button::Left) {
            m_mouseDown = true;
            paintAt(pressed->position);
        }
        return;
    }
    if (const auto* released = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (released->button == sf::Mouse::Button::Left) {
            m_mouseDown = false;
        }
        return;
    }
    if (const auto* moved = event.getIf<sf::Event::MouseMoved>()) {
        m_hoverValid = resolveCell(moved->position, m_hoverRow, m_hoverColumn);
        if (m_mouseDown) {
            paintAt(moved->position);
        }
        return;
    }
    if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        m_scrollX -= wheel->delta * ScrollWheelStep;
        clampScroll();
        return;
    }
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            manager->replaceState(std::make_unique<CustomMapHubState>());
        }
    }
}

void MapEditorState::update(float deltaTime) {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
        m_scrollX -= ScrollKeySpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
        m_scrollX += ScrollKeySpeed * deltaTime;
    }
    clampScroll();

    m_palette.update(deltaTime);
    m_modeToggle.update(deltaTime);
    m_saveButton.update(deltaTime);
    m_backButton.update(deltaTime);
}

void MapEditorState::renderEntityMarkers(sf::RenderTarget& target) const {
    const sf::Font& font = view::AssetManager::instance().getUiFont();
    const float screenW = static_cast<float>(AppEngine::screenWidth());

    const std::size_t firstColumn = static_cast<std::size_t>(m_scrollX / model::TileMap::TileWidth);
    const std::size_t lastColumn = std::min(
        m_map.columns(),
        firstColumn + static_cast<std::size_t>(screenW / model::TileMap::TileWidth) + 2);

    for (std::size_t column = firstColumn; column < lastColumn; ++column) {
        for (std::size_t row = 0; row < model::TileMap::Rows; ++row) {
            const char symbol = m_map.getTile(row, column);
            if (kEntitySymbols.find(symbol) == std::string::npos) {
                continue;
            }

            const float x = static_cast<float>(column * model::TileMap::TileWidth);
            const float y = static_cast<float>((model::TileMap::Rows - 1 - row) * model::TileMap::TileHeight);

            sf::RectangleShape marker(sf::Vector2f{14.f, 14.f});
            marker.setPosition({x + 1.f, y + 1.f});
            marker.setFillColor(sf::Color(0, 0, 0, 170));
            marker.setOutlineColor(symbol == 'M' || symbol == 'E' ? sf::Color::Green : sf::Color::Yellow);
            marker.setOutlineThickness(1.f);
            target.draw(marker);

            sf::Text label(font, std::string(1, symbol), 10u);
            label.setFillColor(sf::Color::White);
            label.setPosition({x + 3.f, y + 1.f});
            target.draw(label);
        }
    }
}

void MapEditorState::renderGhostPreview(sf::RenderTarget& target) const {
    constexpr std::uint8_t GhostAlpha = 130;

    const auto found = m_icons.find(m_selectedSymbol);
    if (found == m_icons.end() || !found->second.texture) {
        return;
    }

    const float x = static_cast<float>(m_hoverColumn * model::TileMap::TileWidth);
    const float y = static_cast<float>((model::TileMap::Rows - 1 - m_hoverRow) * model::TileMap::TileHeight);

    sf::Sprite sprite(*found->second.texture);
    sprite.setTextureRect(found->second.rect);
    sprite.setPosition({x, y});
    sprite.setColor(sf::Color(255, 255, 255, GhostAlpha));
    target.draw(sprite);
}

void MapEditorState::render(sf::RenderTarget& target) {
    const sf::View baseView = target.getView();
    const float screenW = static_cast<float>(AppEngine::screenWidth());
    const float screenH = static_cast<float>(AppEngine::ScreenHeight);

    sf::View gridView = baseView;
    gridView.setSize({screenW, screenH});
    gridView.setCenter({m_scrollX + screenW / 2.f, screenH / 2.f});
    target.setView(gridView);

    target.clear(model::WorldSet::forType(m_map.getWorldType()).getBackgroundColor());

    const model::TileMap tileMap = m_map.toTileMap();
    m_renderer.render(target, tileMap);
    renderEntityMarkers(target);

    // Placement preview: only in PUT mode (index 0), only while hovering the grid, using
    // the same real sprite the placed entity/tile would actually draw as.
    if (m_hoverValid && m_modeToggle.getIndex() == 0) {
        renderGhostPreview(target);
    }

    target.setView(baseView);

    sf::RectangleShape toolbarBg(sf::Vector2f{screenW, ToolbarHeight});
    toolbarBg.setPosition({0.f, screenH - ToolbarHeight});
    toolbarBg.setFillColor(sf::Color(10, 10, 20, 220));
    target.draw(toolbarBg);

    sf::RectangleShape paletteBg(sf::Vector2f{PaletteWidth, screenH - ToolbarHeight});
    paletteBg.setPosition({screenW - PaletteWidth, 0.f});
    paletteBg.setFillColor(sf::Color(10, 10, 20, 220));
    target.draw(paletteBg);

    m_palette.render(target);
    m_modeToggle.render(target);
    m_saveButton.render(target);
    m_backButton.render(target);
    m_statusLabel.render(target);
}

} // namespace controller
