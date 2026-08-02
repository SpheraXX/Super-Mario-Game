#include "Controller/PlayState.h"

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
#include <SFML/Window/Keyboard.hpp>

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

    if (model::GameManager::instance().isGameOver()) {
        manager->replaceState(std::make_unique<GameOverState>());
    }
}

void PlayState::render(sf::RenderWindow& window) {
    const sf::Color skyBlue(92, 148, 252);
    window.clear(skyBlue);

    if (mapLoaded && renderer) {
        renderer->render(window, map);
    }

    sf::Sprite charSprite(charsTexture);
    charSprite.setScale({SpriteScaleX, SpriteScaleY});

    sf::Sprite enemySprite(enemiesTexture);
    enemySprite.setScale({SpriteScaleX, SpriteScaleY});

    sf::Sprite blockSprite(blocksTexture);
    blockSprite.setScale({SpriteScaleX, SpriteScaleY});

    for (const auto& e : entities) {
        if (!e->isActive) continue;

        float snappedX = std::round(e->getPosition().x);
        float snappedY = std::round(e->getPosition().y);

        if (auto* mario = dynamic_cast<model::Mario*>(e.get())) {
            charSprite.setTextureRect({{0, 0}, {16, 32}});
            if (!mario->isFacingRight()) {
                charSprite.setScale({SpriteScaleX, SpriteScaleY});
                charSprite.setOrigin({0.0f, 0.0f});
            } else {
                charSprite.setScale({-SpriteScaleX, SpriteScaleY});
                charSprite.setOrigin({16.0f, 0.0f});
            }
            charSprite.setPosition({snappedX, snappedY});
            window.draw(charSprite);

        } else if (auto* g = dynamic_cast<model::Goomba*>(e.get())) {
            continue;
            // Bug 3 Fix: enemies.png is 436x261. Goomba frames are 16x16 at row 0.
            // Squished goomba (hitbox.isTrigger = true after stomp) uses frame 2.
            if (g->hitbox.isTrigger) {
                // enemySprite.setTextureRect({32, 0, 16, 16});
            } else {
                // enemySprite.setTextureRect({0, 0, 16, 16});
            }
            enemySprite.setScale({SpriteScaleX, SpriteScaleY});
            enemySprite.setPosition({snappedX, snappedY});
            std :: cerr << "Rendering Goomba at (" << snappedX << ", " << snappedY << ")" << std :: endl;
            window.draw(enemySprite);

        } else if (auto* k = dynamic_cast<model::Koopa*>(e.get())) {
            continue;
            // Bug 3 Fix: Koopa walking frames are 16x24 at row 0, col 6 (x=96).
            // Shell idle is 16x16 at row 0, col 10 (x=160).
            if (k->getSize().y <= 16.0f) {
                // enemySprite.setTextureRect({160, 0, 16, 16});
                enemySprite.setScale({SpriteScaleX, SpriteScaleY});
            } else {
                // enemySprite.setTextureRect({96, 0, 16, 24});
                // Koopa is taller: scale Y covers 24 src px → 1.5 world tiles
                enemySprite.setScale({SpriteScaleX,
                    static_cast<float>(model::TileMap::TileHeight) * 1.5f / 24.0f});
            }
            enemySprite.setPosition({snappedX, snappedY});
            std :: cerr << "Rendering Koopa at (" << snappedX << ", " << snappedY << ")" << std :: endl;
            window.draw(enemySprite);

        } else if (dynamic_cast<model::CoinBlock*>(e.get())) {
            continue;
            // Bug 5 Fix: Use the real block texture (question block = col 0, row 0 of blocks.png).
            // blocks.png tiles are the same 16px source size as everything else.
            // blockSprite.setTextureRect({0, 0, 16, 16});
            blockSprite.setPosition({snappedX, snappedY});
            std :: cerr << "Rendering Coin Block at (" << snappedX << ", " << snappedY << ")" << std :: endl;
            window.draw(blockSprite);
        }
    }

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
        hint.setPosition({10.f, static_cast<float>(window.getSize().y) - 26.f});
        window.draw(hint);
    }
}

}
