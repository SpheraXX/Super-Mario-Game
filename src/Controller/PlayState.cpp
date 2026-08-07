#include "Controller/PlayState.h"

#include "Controller/AppEngine.h"
#include "Controller/GameOverState.h"
#include "Controller/LevelCompleteState.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/Block/BrickBlock.h"
#include "Model/Block/CoinBlock.h"
#include "Model/Character.h"
#include "Model/Core/GameManager.h"
#include "Model/Enemy/Goomba.h"
#include "Model/Enemy/Koopa.h"
#include "Model/Level/Castle.h"
#include "Model/Level/FlagPole.h"
#include "Model/Player/Luigi.h"
#include "Model/Player/Mario.h"
#include "Model/Player/Player.h"
#include "Model/World/WorldSet.h"
#include "View/Block/BrickBlockRenderer.h"
#include "View/Block/CoinBlockRenderer.h"
#include "View/Enemy/GoombaRenderer.h"
#include "View/Enemy/KoopaRenderer.h"
#include "View/Level/CastleRenderer.h"
#include "View/Level/FlagPoleRenderer.h"
#include "View/Player/PlayerRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

namespace controller {

namespace {
// The level-completion zone appended to every map: 16 empty columns holding the
// flagpole (6 tiles into the zone) and the castle (13 tiles in, 3 tiles wide).
constexpr std::size_t LevelPaddingTiles = 16;
constexpr std::size_t PoleOffsetTiles = 6;
constexpr std::size_t CastleOffsetTiles = 13;

constexpr float TimerStartSeconds = 400.0f;
constexpr int FlagBonus = 5000;
constexpr int TimeBonusPerSecond = 10;

// TEMP trace instrumentation (removed after playtest).
void trace(const std::string& msg) {
    std::ofstream out("trace_log.txt", std::ios::app);
    out << msg << '\n';
}

bool isGroundSymbol(char symbol) {
    return symbol == 'G' || symbol == 'C' || symbol == 'B' || symbol == '#';
}

// Top face of the ground stack at the given column: scan up from the bottom row and
// return the top edge of the contiguous solid base. Falls back to a standard height.
float groundTopAt(const model::TileMap& map, std::size_t column) {
    const std::size_t rows = map.getRows();
    if (column < map.getColumns()) {
        std::size_t solid = 0;
        while (solid < rows && isGroundSymbol(map.getTile(solid, column))) {
            ++solid;
        }
        if (solid > 0) {
            return static_cast<float>((rows - 1 - (solid - 1)) * model::TileMap::TileHeight);
        }
    }
    return static_cast<float>((rows - 2) * model::TileMap::TileHeight);
}
}

void PlayState::onEnter() {
    auto& game = model::GameManager::instance();
    try {
        map.loadFromFile(game.getCurrentMapPath());
        map.padRight(LevelPaddingTiles);
        renderer = std::make_unique<view::TileMapRenderer>("assets/blocks.png", map.getWorldType());
        mapLoaded = true;
    } catch (const std::exception& error) {
        std::cerr << "PlayState: failed to load level assets: " << error.what() << '\n';
        mapLoaded = false;
    }

    worldType = map.getWorldType();
    timer.reset(TimerStartSeconds);
    levelComplete = false;

    // Publish the map's metadata where the HUD and the completion flow read it.
    game.setLevelName(map.getLevelName());
    game.setNextMapPath(map.getNextMapPath());

    // Build the view: one renderer per entity type + the screen-space HUD.
    entityRenderers = std::make_unique<view::EntityRendererRegistry>();
    entityRenderers->registerRenderer<model::Mario, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Luigi, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Goomba, view::GoombaRenderer>();
    entityRenderers->registerRenderer<model::Koopa, view::KoopaRenderer>();
    entityRenderers->registerRenderer<model::CoinBlock, view::CoinBlockRenderer>();
    entityRenderers->registerRenderer<model::BrickBlock, view::BrickBlockRenderer>();
    entityRenderers->registerRenderer<model::FlagPole, view::FlagPoleRenderer>();
    entityRenderers->registerRenderer<model::Castle, view::CastleRenderer>();
    hudRenderer = std::make_unique<view::HudRenderer>();

    // Initialize collision manager
    collisionManager = std::make_unique<model::CollisionManager>(&map);

    // Spawn the initial set of entities (also used for respawns after a death).
    resetLevel();

    // TEMP diagnostics (removed after playtest).
    if (map.getColumns() > 0) {
        trace("mapLoad cols=" + std::to_string(map.getColumns())
              + " g00=" + std::string(1, map.getTile(0, 0))
              + " m26=" + std::string(1, map.getTile(2, 6))
              + " loaded=" + std::to_string(mapLoaded)
              + " world=" + std::to_string(static_cast<int>(worldType)));
    } else {
        trace("mapLoad FAILED loaded=" + std::to_string(mapLoaded));
    }
}

// (Re)build the entity list from scratch: the map file drives what spawns where.
// 'M' = Mario, 'E' = Goomba, 'K' = Koopa, 'C' = CoinBlock, '#'/'B' = BrickBlock.
// Every entity spawns exactly at its cell (row 0 is the bottom row), so the map
// encodes both position and height precisely — no support scan, no surprises.
// Called on enter and after every death (the whole level restarts).
void PlayState::resetLevel() {
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const std::size_t tileHeight = model::TileMap::TileHeight;
    const std::size_t rows = map.getRows();
    const std::size_t columns = map.getColumns();

    trace("resetLevel");
    entities.clear();
    player = nullptr;
    flagPole = nullptr;

    bool marioSpawned = false;
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const char symbol = map.getTile(row, column);
            const model::Vector2 position{static_cast<float>(column * tileWidth),
                                          static_cast<float>((rows - 1 - row) * tileHeight)};
            const model::Vector2 size{static_cast<float>(tileWidth),
                                      static_cast<float>(tileHeight)};

            switch (symbol) {
                case 'M':
                    if (!marioSpawned) {
                        trace("spawn " + std::string(typeid(model::Mario).name()) + " "
                              + std::to_string(position.x) + " " + std::to_string(position.y));
                        auto mario = std::make_unique<model::Mario>(position);
                        player = mario.get();
                        entities.push_back(std::move(mario));
                        marioSpawned = true;
                    }
                    break;
                case 'E': {
                    trace("spawn " + std::string(typeid(model::Goomba).name()) + " "
                          + std::to_string(position.x) + " " + std::to_string(position.y));
                    auto goomba = std::make_unique<model::Goomba>(position);
                    goomba->setMap(&map);  // for ledge detection
                    entities.push_back(std::move(goomba));
                    break;
                }
                case 'K': {
                    trace("spawn " + std::string(typeid(model::Koopa).name()) + " "
                          + std::to_string(position.x) + " " + std::to_string(position.y));
                    auto koopa = std::make_unique<model::Koopa>(position);
                    koopa->setMap(&map);  // for ledge detection
                    entities.push_back(std::move(koopa));
                    break;
                }
                case 'C':
                    trace("spawn " + std::string(typeid(model::CoinBlock).name()) + " "
                          + std::to_string(position.x) + " " + std::to_string(position.y));
                    entities.push_back(std::make_unique<model::CoinBlock>(position, size));
                    break;
                case '#':
                case 'B':
                    trace("spawn " + std::string(typeid(model::BrickBlock).name()) + " "
                          + std::to_string(position.x) + " " + std::to_string(position.y));
                    entities.push_back(std::make_unique<model::BrickBlock>(position, size));
                    break;
                default:
                    break;
            }
        }
    }

    // Fallback: if the map has no 'M', keep the game playable with a fixed spawn.
    if (!marioSpawned) {
        const float groundY = static_cast<float>((rows - 2) * tileHeight - tileHeight);
        auto mario = std::make_unique<model::Mario>(model::Vector2{64.0f, groundY});
        player = mario.get();
        entities.push_back(std::move(mario));
    }

    // Level completion zone, in the padded columns: flagpole, then the goal castle.
    // (Guard: with a failed load columns is 0 and there is nothing to spawn.)
    if (columns < LevelPaddingTiles) {
        return;
    }
    const std::size_t baseColumns = columns - LevelPaddingTiles;
    const float groundTop = groundTopAt(map, baseColumns > 0 ? baseColumns - 1 : 0);
    const float poleHeight = 224.0f;
    const float castleHeight = 160.0f;

    auto flag = std::make_unique<model::FlagPole>(
        model::Vector2{static_cast<float>((baseColumns + PoleOffsetTiles) * tileWidth),
                       groundTop - poleHeight},
        model::Vector2{8.0f, poleHeight});
    flagPole = flag.get();
    entities.push_back(std::move(flag));

    entities.push_back(std::make_unique<model::Castle>(
        model::Vector2{static_cast<float>((baseColumns + CastleOffsetTiles) * tileWidth),
                       groundTop - castleHeight},
        model::Vector2{3.0f * tileWidth, castleHeight}));

    // Every character obeys the current world's physics (gravity/fall/drag, swim).
    const model::World& world = model::WorldSet::forType(worldType);
    for (const auto& e : entities) {
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            character->setWorld(world);
        }
    }
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
    // Once the flag is touched the level is frozen behind the LevelComplete overlay:
    // no timer, no input, no physics.
    if (levelComplete) {
        return;
    }

    // SMB timer: one tick per second. Running out of time is a death.
    if (player && !player->isDying()) {
        timer.update(deltaTime);
        if (timer.isExpired()) {
            player->die(true);
        }
    }

    std::vector<model::Entity*> activeEntities;
    for (auto& e : entities) {
        if (!e->isActive) continue;

        // Input gathering is delegated polymorphically: only the player reacts.
        // It runs BEFORE entity->update() so gravity & integration see the correct
        // player-intended velocity, not stale values.
        e->handleInput(deltaTime);

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

        // TEMP diagnostics (removed after playtest).
        static int failFrame = 0;
        if (!mapLoaded && ++failFrame % 10 == 0 && e.get() == player) {
            trace("fail pos=" + std::to_string(pos.x) + "," + std::to_string(pos.y)
                  + " g=" + std::to_string(e->isGrounded)
                  + " vy=" + std::to_string(e->getVelocity().y));
        }

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
            // The FEET decide: the y-clamp below pins the player to mapHeight - size.y,
            // so checking the top (pos.y) can never be exceeded and the death would
            // never trigger.
            if (pos.y + sz.y >= mapHeight) {
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

    // Flagpole touch: award the clear bonus (flag height + time remaining) and push the
    // transparent completion overlay; update() above returns from now on, freezing the
    // level behind it.
    if (flagPole && flagPole->isTouched() && player && !player->isDying()) {
        levelComplete = true;
        timer.pause();
        const int timeBonus = timer.getRemainingSeconds() * TimeBonusPerSecond;
        model::GameManager::instance().addScore(FlagBonus + timeBonus);
        model::GameManager::instance().setLevelClearBonus(FlagBonus + timeBonus);
        trace("levelComplete bonus=" + std::to_string(FlagBonus + timeBonus));
        manager->pushState(std::make_unique<LevelCompleteState>());
    }

    // HUD snapshot for the next frame.
    auto& game = model::GameManager::instance();
    hudData.score = game.getScore();
    hudData.coins = game.getCoins();
    hudData.levelName = game.getLevelName();
    hudData.time = timer.getRemainingSeconds();
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

    // The world's theme decides the background.
    window.clear(model::WorldSet::forType(worldType).getBackgroundColor());

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
        hudRenderer->render(window, hudData);
    }
}

}
