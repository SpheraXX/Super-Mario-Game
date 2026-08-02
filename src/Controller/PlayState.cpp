#include "Controller/PlayState.h"

#include "Controller/AppEngine.h"
#include "Controller/GameOverState.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/GameManager.h"
#include "Model/Mario.h"
#include "Model/Player.h"
#include "Model/Goomba.h"
#include "Model/Koopa.h"
#include "Model/CoinBlock.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <string>

namespace controller {

namespace {
std::string mapPathForLevel(int level) {
    (void)level;
    return "assets/maps/debug.map";
}

constexpr float SpriteScaleX = static_cast<float>(model::TileMap::TileWidth)  / 16.0f;
constexpr float SpriteScaleY = static_cast<float>(model::TileMap::TileHeight) / 16.0f;

// Source frames are assumed to be 16x32 (width x height). The atlas coordinates below are
// PLACEHOLDERS so each entity picks a distinct frame — update them once the real
// spritesheet layout is finalised by the graphics teammate.
constexpr int MarioFrameCol = 0;
constexpr int GoombaFrameCol = 0;
constexpr int GoombaSquishedFrameCol = 1;
constexpr int KoopaFrameCol = 2;
constexpr int KoopaShellFrameCol = 3;

// Configure a character sprite for one entity:
//  - the 16x32 source frame is scaled up to one world tile per 16 source pixels;
//  - the origin is lowered so the frame's bottom edge (the character's feet) sits exactly
//    on the entity's bottom edge, instead of the sprite hanging below the hitbox;
//  - the frame is mirrored horizontally when the entity faces left.
void setupEntitySprite(sf::Sprite& sprite, const model::Vector2& entitySize, bool facingRight) {
    const float originY = 32.0f - entitySize.y / SpriteScaleY;
    if (facingRight) {
        sprite.setScale({-SpriteScaleX, SpriteScaleY});
        sprite.setOrigin({16.0f, originY});
    } else {
        sprite.setScale({SpriteScaleX, SpriteScaleY});
        sprite.setOrigin({0.0f, originY});
    }
}
}

void PlayState::onEnter() {
    const int level = model::GameManager::instance().getCurrentLevel();
    try {
        map.loadFromFile(mapPathForLevel(level));
        renderer = std::make_unique<view::TileMapRenderer>("assets/blocks.png");
        mapLoaded = true;
    } catch (const std::exception& error) {
        std::cerr << "PlayState: failed to load level assets: " << error.what() << '\n';
        mapLoaded = false;
    }

    fontLoaded = font.openFromFile("assets/fonts/Tuffy.ttf");
    
    // Load entity textures
    if (!charsTexture.loadFromFile("assets/characters.png")) {
        std::cerr << "Failed to load characters.png\n";
    }
    charsTexture.setSmooth(false);
    
    if (!enemiesTexture.loadFromFile("assets/enemies.png")) {
        std::cerr << "Failed to load enemies.png\n";
    }
    enemiesTexture.setSmooth(false);

    // Initialize collision manager
    collisionManager = std::make_unique<model::CollisionManager>(&map);

    // Spawn Player — place on ground row (row 1 = y = (Rows-2)*TileHeight)
    // Ground is at tilemap rows 0-1 (bottom). In world coords bottom row 0 is at
    // y = (Rows-1)*TileHeight. Spawn Mario one tile above ground.
    const float groundY = static_cast<float>((model::TileMap::Rows - 2) * model::TileMap::TileHeight
                                             - model::TileMap::TileHeight);
    auto mario = std::make_unique<model::Mario>(model::Vector2{64.0f, groundY});
    player = mario.get();
    entities.push_back(std::move(mario));

    // Spawn Hostiles — also on the ground level, to the right of Mario
    auto goomba = std::make_unique<model::Goomba>(model::Vector2{512.0f, groundY});
    entities.push_back(std::move(goomba));

    auto koopa = std::make_unique<model::Koopa>(model::Vector2{768.0f, groundY});
    entities.push_back(std::move(koopa));

    // Bug 5 Fix: Spawn CoinBlock using block texture, not a red rect.
    // Position it 5 tiles above ground in world coords.
    const float blockY = groundY - 5.0f * model::TileMap::TileHeight;
    auto coinBlock = std::make_unique<model::CoinBlock>(
        model::Vector2{320.0f, blockY},
        model::Vector2{16.0f, 16.0f});
    entities.push_back(std::move(coinBlock));

    // Also load block texture for CoinBlock rendering
    if (!blocksTexture.loadFromFile("assets/blocks.png")) {
        std::cerr << "Failed to load blocks.png\n";
    }
    blocksTexture.setSmooth(false);
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case sf::Keyboard::Key::Escape:
                manager->replaceState(std::make_unique<MenuState>());
                break;
            case sf::Keyboard::Key::G:
                model::GameManager::instance().addScore(500);
                model::GameManager::instance().loseLife();
                model::GameManager::instance().loseLife();
                model::GameManager::instance().loseLife();
                break;
            default:
                break;
        }
    }
}

void PlayState::update(float deltaTime) {
    std::vector<model::Entity*> activeEntities;
    for (auto& e : entities) {
        if (!e->isActive) continue;

        // Bug 2 Fix: delegate all input reading to Player::handleInput().
        // handleInput() sets velocity.x and fires jump via velocity.y.
        // It runs BEFORE entity->update() so gravity & integration see
        // the correct player-intended velocity, not stale values.
        if (auto* p = dynamic_cast<model::Player*>(e.get())) {
            p->handleInput();
        }

        e->update(deltaTime);
        activeEntities.push_back(e.get());
    }

    if (collisionManager) {
        collisionManager->update(activeEntities, deltaTime);
    }

    // Illegal to leave the map: clamp every entity inside its bounds (both side edges
    // and the top/bottom, e.g. so a fall into a pit cannot leave the world).
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float mapHeight = static_cast<float>(map.getRows()) * model::TileMap::TileHeight;
    for (const auto& e : entities) {
        if (!e->isActive) continue;
        model::Vector2 pos = e->getPosition();
        const model::Vector2 sz = e->getSize();
        pos.x = std::clamp(pos.x, 0.0f, std::max(0.0f, mapWidth - sz.x));
        pos.y = std::clamp(pos.y, 0.0f, std::max(0.0f, mapHeight - sz.y));
        e->setPosition(pos);
    }

    if (model::GameManager::instance().isGameOver()) {
        manager->replaceState(std::make_unique<GameOverState>());
    }
}

void PlayState::render(sf::RenderWindow& window) {
    // Camera: follows the player horizontally, but never pans past the map fringes; it is
    // fixed vertically. The view keeps the letterbox viewport set by AppEngine.
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float halfWidth = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    float cameraX = halfWidth;
    if (player) {
        cameraX = player->getPosition().x + player->getSize().x / 2.0f;
    }
    cameraX = std::clamp(cameraX, halfWidth, std::max(halfWidth, mapWidth - halfWidth));

    const sf::View baseView = window.getView();
    sf::View cameraView = baseView;
    cameraView.setSize({static_cast<float>(AppEngine::ScreenWidth),
                        static_cast<float>(AppEngine::ScreenHeight)});
    cameraView.setCenter({cameraX, static_cast<float>(AppEngine::ScreenHeight) / 2.0f});
    window.setView(cameraView);

    const sf::Color skyBlue(92, 148, 252);
    window.clear(skyBlue);

    if (mapLoaded && renderer) {
        renderer->render(window, map);
    }

    sf::Sprite charSprite(charsTexture);
    sf::Sprite enemySprite(enemiesTexture);
    sf::Sprite blockSprite(blocksTexture);

    for (const auto& e : entities) {
        if (!e->isActive) continue;

        float snappedX = std::round(e->getPosition().x);
        float snappedY = std::round(e->getPosition().y);

        if (auto* mario = dynamic_cast<model::Mario*>(e.get())) {
            charSprite.setTextureRect({{MarioFrameCol * 16, 0}, {16, 32}});
            setupEntitySprite(charSprite, mario->getSize(), mario->isFacingRight());
            charSprite.setPosition({snappedX, snappedY});
            window.draw(charSprite);

        } else if (auto* g = dynamic_cast<model::Goomba*>(e.get())) {
            if (g->hitbox.isTrigger) {
                // Squished Goomba: placeholder next frame in the sheet.
                enemySprite.setTextureRect({{GoombaSquishedFrameCol * 16, 0}, {16, 32}});
            } else {
                enemySprite.setTextureRect({{GoombaFrameCol * 16, 0}, {16, 32}});
            }
            setupEntitySprite(enemySprite, g->getSize(), g->isFacingRight());
            enemySprite.setPosition({snappedX, snappedY});
            window.draw(enemySprite);

        } else if (auto* k = dynamic_cast<model::Koopa*>(e.get())) {
            if (k->getSize().y <= 16.0f) {
                // Shell: placeholder next frame in the sheet.
                enemySprite.setTextureRect({{KoopaShellFrameCol * 16, 0}, {16, 32}});
            } else {
                enemySprite.setTextureRect({{KoopaFrameCol * 16, 0}, {16, 32}});
            }
            setupEntitySprite(enemySprite, k->getSize(), k->isFacingRight());
            enemySprite.setPosition({snappedX, snappedY});
            window.draw(enemySprite);

        } else if (dynamic_cast<model::CoinBlock*>(e.get())) {
            // Block tiles in blocks.png are 16x16 source pixels (one tile), unlike the
            // 16x32 character frames. (5, 7) is the same coin-block tile the map renderer
            // uses for 'C' tiles, so the entity matches the level art.
            blockSprite.setTextureRect({{5 * 16, 7 * 16}, {16, 16}});
            blockSprite.setScale({SpriteScaleX, SpriteScaleY});
            blockSprite.setOrigin({0.0f, 0.0f});
            blockSprite.setPosition({snappedX, snappedY});
            window.draw(blockSprite);
        }
    }

    // Restore the fixed (non-scrolling) view so HUD text stays on the screen.
    window.setView(baseView);

    if (fontLoaded) {
        sf::Text levelLabel(font, "DEBUG LEVEL", 22);
        levelLabel.setFillColor(sf::Color::White);
        levelLabel.setOutlineColor(sf::Color::Black);
        levelLabel.setOutlineThickness(2.f);
        levelLabel.setPosition({10.f, 8.f});
        window.draw(levelLabel);

        sf::Text hint(font, "WASD/Arrows: Move  |  Space/W/Up: Jump  |  ESC: Menu", 16);
        hint.setFillColor(sf::Color::White);
        hint.setOutlineColor(sf::Color::Black);
        hint.setOutlineThickness(2.f);
        hint.setPosition({10.f, static_cast<float>(AppEngine::ScreenHeight) - 26.f});
        window.draw(hint);
    }
}

}
