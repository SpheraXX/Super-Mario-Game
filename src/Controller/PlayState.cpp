#include "Controller/PlayState.h"

#include "Controller/AppEngine.h"
#include "Controller/GameOverState.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Player/Mario.h"
#include "Model/Player/Luigi.h"
#include "Model/Player/Player.h"
#include "Model/Enemy/Goomba.h"
#include "Model/Enemy/Koopa.h"
#include "Model/Block/CoinBlock.h"
#include "View/Block/CoinBlockRenderer.h"
#include "View/Enemy/GoombaRenderer.h"
#include "View/Enemy/KoopaRenderer.h"
#include "View/Player/PlayerRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
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

    // Build the view: one renderer per entity type + the screen-space HUD.
    entityRenderers = std::make_unique<view::EntityRendererRegistry>();
    entityRenderers->registerRenderer<model::Mario, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Luigi, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Goomba, view::GoombaRenderer>();
    entityRenderers->registerRenderer<model::Koopa, view::KoopaRenderer>();
    entityRenderers->registerRenderer<model::CoinBlock, view::CoinBlockRenderer>();
    hudRenderer = std::make_unique<view::HudRenderer>();

    // Initialize collision manager
    collisionManager = std::make_unique<model::CollisionManager>(&map);

    // Spawn the initial set of entities (also used for respawns after a death).
    resetLevel();
}

// (Re)build the entity list from scratch: a fresh Mario plus the level's enemies and
// blocks. Called on enter and after every death (the whole level restarts).
void PlayState::resetLevel() {
    int TileHeight = model::TileMap::TileHeight;
    entities.clear();
    player = nullptr;

    // Spawn Player — place on ground row (row 1 = y = (Rows-2)*TileHeight)
    // Ground is at tilemap rows 0-1 (bottom). In world coords bottom row 0 is at
    // y = (Rows-1)*TileHeight. Spawn Mario one tile above ground.
    const float groundY = static_cast<float>((model::TileMap::Rows - 2) * TileHeight
                                             - TileHeight);
    auto mario = std::make_unique<model::Mario>(model::Vector2{64.0f, groundY});
    player = mario.get();
    entities.push_back(std::move(mario));

    // Spawn Hostiles — also on the ground level, to the right of Mario
    auto goomba = std::make_unique<model::Goomba>(model::Vector2{512.0f, groundY});
    entities.push_back(std::move(goomba));

    auto koopa = std::make_unique<model::Koopa>(model::Vector2{768.0f, groundY});
    entities.push_back(std::move(koopa));

    // Spawn CoinBlock using block texture, not a red rect.
    // Position it 5 tiles above ground in world coords. One world tile in size so its
    // hitbox matches the tile art it renders with.
    const float blockY = groundY - 5.0f * TileHeight;
    auto coinBlock = std::make_unique<model::CoinBlock>(
        model::Vector2{320.0f, blockY},
        model::Vector2{static_cast<float>(model::TileMap::TileWidth),
                       static_cast<float>(model::TileMap::TileHeight)});
    entities.push_back(std::move(coinBlock));
}

void PlayState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        switch (key->code) {
            case sf::Keyboard::Key::Escape:
                manager->replaceState(std::make_unique<MenuState>());
                break;
            case sf::Keyboard::Key::G:
                // Debug: kill the player through the normal death flow.
                if (player && !player->isDying()) {
                    player->die(true);
                }
                break;
            case sf::Keyboard::Key::H:
                // Debug: toggle the collision-box overlay.
                showHitboxes = !showHitboxes;
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

        // Input gathering is delegated polymorphically: only the player reacts.
        // It runs BEFORE entity->update() so gravity & integration see the correct
        // player-intended velocity, not stale values.
        e->handleInput();

        e->update(deltaTime);
        activeEntities.push_back(e.get());
    }

    if (collisionManager) {
        collisionManager->update(activeEntities, deltaTime);
    }

    // World bounds handling: the player stays inside the map (and falling past the
    // bottom is a pit death); enemies despawn once they leave the world. Dying bodies
    // ignore the bounds entirely and fall away until they are removed.
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float mapHeight = static_cast<float>(map.getRows()) * model::TileMap::TileHeight;

    bool playerFinishedDeathFall = false;
    for (const auto& e : entities) {
        if (!e->isActive) continue;
        model::Vector2 pos = e->getPosition();
        const model::Vector2 sz = e->getSize();

        // Bodies that finished their (non-animated) death are gone for good, e.g.
        // squished Goombas after their despawn timer.
        if (!e->isAlive() && !e->isDying()) {
            e->isActive = false;
            continue;
        }

        // Dying bodies fall through the world; once past the bottom they are removed.
        if (e->isDying()) {
            if (pos.y > mapHeight) {
                if (e.get() == player) {
                    playerFinishedDeathFall = true;
                }
                e->isActive = false;
            }
            continue;
        }

        if (e.get() == player) {
            // The player cannot leave the map; a fall past the bottom is a pit death.
            if (pos.y > mapHeight) {
                player->die(false); // no bounce: the body just keeps dropping
                continue;
            }
            pos.x = std::clamp(pos.x, 0.0f, std::max(0.0f, mapWidth - sz.x));
            pos.y = std::clamp(pos.y, 0.0f, std::max(0.0f, mapHeight - sz.y));
            e->setPosition(pos);
        } else {
            // Hostiles/others: despawn once they leave the world bounds (walked off
            // the map edge, which is also off camera, or fell into a pit).
            if (pos.x + sz.x < 0.0f || pos.x > mapWidth || pos.y > mapHeight) {
                e->isActive = false;
            }
        }
    }

    // The player's death fall is over: either the run is over or the level restarts.
    if (playerFinishedDeathFall) {
        if (model::GameManager::instance().isGameOver()) {
            manager->replaceState(std::make_unique<GameOverState>());
        } else {
            resetLevel();
        }
    }
}

void PlayState::render(sf::RenderTarget& window) {
    // Camera: follows the player horizontally, but never pans past the map fringes; it is
    // fixed vertically. The view keeps the fixed viewport set by AppEngine.
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float halfWidth = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    float cameraX = halfWidth;
    if (player) {
        cameraX = player->getPosition().x + player->getSize().x / 2.0f;
    }
    cameraX = std::clamp(cameraX, halfWidth, std::max(halfWidth, mapWidth - halfWidth));
    // Snap the camera to a whole logical pixel. The world is composited into an offscreen
    // target at the logical resolution and upscaled once, so logical pixels *are* the grid
    // that matters here: integer camera positions keep every tile edge aligned (no seams)
    // while the scroll rate stays perfectly even (WindowScale never enters this maths).
    cameraX = std::round(cameraX);

    const sf::View baseView = window.getView();
    sf::View cameraView = baseView;
    cameraView.setSize({static_cast<float>(AppEngine::ScreenWidth),
                        static_cast<float>(AppEngine::ScreenHeight)});
    cameraView.setCenter({cameraX, static_cast<float>(AppEngine::ScreenHeight) / 2.0f});
    window.setView(cameraView);

    const sf::Color skyBlue(92, 148, 252);
    window.clear(skyBlue);

    // World space: the tile map, then every active entity through its registered
    // renderer (no type checks here — the view dispatches polymorphically).
    if (mapLoaded && renderer) {
        renderer->render(window, map);
    }
    if (entityRenderers) {
        for (const auto& e : entities) {
            if (e->isActive) {
                entityRenderers->render(window, *e);
            }
        }
    }

    // Debug overlay: hitboxes go on top of the sprites, still in world space so they line
    // up with what they bound. Solid tiles first, then entities over them.
    if (showHitboxes) {
        if (mapLoaded) {
            hitboxRenderer.renderTiles(window, map);
        }
        for (const auto& e : entities) {
            if (e->isActive) {
                hitboxRenderer.render(window, *e);
            }
        }
    }

    // Restore the fixed (non-scrolling) view so HUD text stays on the screen.
    window.setView(baseView);

    if (hudRenderer) {
        hudRenderer->render(window);
    }
}

}
