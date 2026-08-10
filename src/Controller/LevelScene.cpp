#include "Controller/LevelScene.h"

#include "Controller/AppEngine.h"
#include "Model/Block/BrickBlock.h"
#include "Model/Block/CoinBlock.h"
#include "Model/Character.h"
#include "Model/Core/GameManager.h"
#include "Model/Enemy/Goomba.h"
#include "Model/Enemy/Koopa.h"
#include "Model/Level/FlagPole.h"
#include "Model/Level/Pipe.h"
#include "Model/Player/Luigi.h"
#include "Model/Player/Mario.h"
#include "Model/Player/Player.h"
#include "Model/World/WorldSet.h"
#include "View/Base/RenderContext.h"
#include "View/Block/BrickBlockRenderer.h"
#include "View/Block/CoinBlockRenderer.h"
#include "View/Enemy/GoombaRenderer.h"
#include "View/Enemy/KoopaRenderer.h"
#include "View/Level/FlagPoleRenderer.h"
#include "View/Level/PipeRenderer.h"
#include "View/Player/PlayerRenderer.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

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
// flagpole (6 tiles into the zone) and the painted castle (11 tiles in, 5 tiles wide).
constexpr std::size_t LevelPaddingTiles = 16;
constexpr std::size_t PoleOffsetTiles = 6;
constexpr std::size_t CastleOffsetTiles = 11;

constexpr float TimerStartSeconds = 400.0f;

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

LevelScene::LevelScene()
    : entityRenderers(std::make_unique<view::EntityRendererRegistry>()),
      collisionManager(std::make_unique<model::CollisionManager>(&map)) {
    // Build the view: one renderer per entity type (the tile renderer is rebuilt per
    // area in loadArea; the collision manager resolves against the working map).
    entityRenderers->registerRenderer<model::Mario, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Luigi, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Goomba, view::GoombaRenderer>();
    entityRenderers->registerRenderer<model::Koopa, view::KoopaRenderer>();
    entityRenderers->registerRenderer<model::CoinBlock, view::CoinBlockRenderer>();
    entityRenderers->registerRenderer<model::BrickBlock, view::BrickBlockRenderer>();
    entityRenderers->registerRenderer<model::FlagPole, view::FlagPoleRenderer>();
    entityRenderers->registerRenderer<model::Pipe, view::PipeRenderer>();
}

bool LevelScene::loadLevel() {
    auto& game = model::GameManager::instance();
    try {
        level.loadFromFile(game.getCurrentMapPath());
        loadArea(0);
    } catch (const std::exception& error) {
        std::cerr << "LevelScene: failed to load level assets: " << error.what() << '\n';
        mapLoaded = false;
        return false;
    }

    // Publish the level's metadata where the HUD and the completion flow read it.
    game.setLevelName(level.getLevelName());
    game.setNextMapPath(level.getNextMapPath());

    timer.reset(TimerStartSeconds);

    // TEMP diagnostics (removed after playtest).
    if (mapLoaded && map.getColumns() > 0) {
        trace("mapLoad cols=" + std::to_string(map.getColumns())
              + " g00=" + std::string(1, map.getTile(0, 0))
              + " m26=" + std::string(1, map.getTile(2, 6))
              + " loaded=" + std::to_string(mapLoaded)
              + " world=" + std::to_string(static_cast<int>(worldType)));
    } else {
        trace("mapLoad FAILED loaded=" + std::to_string(mapLoaded));
    }
    return true;
}

// Instantiate the given area: copy its grid into the working map, rebuild the themed
// renderer, append the completion zone on the FINAL area only, then spawn the area.
void LevelScene::loadArea(std::size_t areaIndex) {
    currentArea = areaIndex;
    portals.clear();  // every visit to an area reactivates all its pipes
    worldType = level.areaWorld(areaIndex);
    map = level.areaMap(areaIndex);
    if (currentArea == level.areaCount() - 1) {
        map.padRight(LevelPaddingTiles);
    }
    renderer = std::make_unique<view::TileMapRenderer>("assets/blocks.png", worldType);
    mapLoaded = true;
    resetLevel();
}

void LevelScene::teleportToPortal(const model::Portal& portal) {
    if (portal.destinationArea >= level.areaCount()) {
        return;
    }

    // Rebuild the destination area and its entities, then place Mario either on the
    // cap of the destination pipe (if the arrival column has one) or on the ground.
    // The camera, HUD and timer all keep their state.
    loadArea(portal.destinationArea);
    portals.markInert(portal.destinationColumn);  // one-way: no re-entry here
    if (!playerPtr) {
        return;
    }
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const float landY = portals.landingY(map, entities, portal.destinationColumn,
                                         playerPtr->getSize().y);
    playerPtr->setPosition({
        static_cast<float>(portal.destinationColumn * tileWidth),
        landY});
    playerPtr->setVelocity({0.0f, 0.0f});
}

// (Re)build the entity list from scratch: the map file drives what spawns where.
// 'M' = Mario, 'E' = Goomba, 'K' = Koopa, 'C' = CoinBlock, '#'/'B' = BrickBlock.
// Every entity spawns exactly at its cell (row 0 is the bottom row), so the map
// encodes both position and height precisely — no support scan, no surprises.
// Called on enter and after every death (the whole level restarts).
void LevelScene::resetLevel() {
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const std::size_t tileHeight = model::TileMap::TileHeight;
    const std::size_t rows = map.getRows();
    const std::size_t columns = map.getColumns();

    trace("resetLevel");
    entities.clear();
    playerPtr = nullptr;
    flagPolePtr = nullptr;

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
                        playerPtr = mario.get();
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

    // Pipes: contiguous vertical runs of 'P' (and, temporarily, 'p') on one column
    // become a single Pipe whose box covers the whole run (cap on the top cell, plain
    // body below). 'P' marks the LEFT column of a 2-tile-wide pipe: the cell to its
    // right must stay empty ('.'/'-'), so the pipe spans two tiles while the map only
    // encodes its left column. If the right-hand cells are occupied the run falls back
    // to a 1-wide pipe to stay playable. The column is what links the entity to its
    // level portal, if any.
    for (std::size_t column = 0; column < columns; ++column) {
        std::size_t runStart = 0;
        while (runStart < rows) {
            while (runStart < rows && map.getTile(runStart, column) != 'P'
                   && map.getTile(runStart, column) != 'p') {
                ++runStart;
            }
            if (runStart >= rows) {
                break;
            }
            std::size_t runEnd = runStart;
            while (runEnd + 1 < rows && (map.getTile(runEnd + 1, column) == 'P'
                   || map.getTile(runEnd + 1, column) == 'p')) {
                ++runEnd;
            }
            // Row 0 is the bottom row; the cap is the topmost row of the run.
            const float pipeTop = static_cast<float>((rows - 1 - runEnd) * tileHeight);
            const float pipeHeight = static_cast<float>((runEnd - runStart + 1) * tileHeight);
            bool wide = column + 1 < columns;
            if (wide) {
                for (std::size_t row = runStart; row <= runEnd && wide; ++row) {
                    const char rightCell = map.getTile(row, column + 1);
                    wide = (rightCell == '.' || rightCell == '-');
                }
                if (!wide) {
                    trace("pipe at column " + std::to_string(column) +
                          ": right cell not empty, spawning 1-wide");
                }
            }
            const float pipeWidth =
                wide ? 2.0f * static_cast<float>(tileWidth) : static_cast<float>(tileWidth);
            auto pipe = std::make_unique<model::Pipe>(
                model::Vector2{static_cast<float>(column * tileWidth), pipeTop},
                model::Vector2{pipeWidth, pipeHeight}, column);
            entities.push_back(std::move(pipe));
            runStart = runEnd + 1;
        }
    }

    // Fallback: if the map has no 'M', keep the game playable with a fixed spawn.
    if (!marioSpawned) {
        const float groundY = static_cast<float>((rows - 2) * tileHeight - tileHeight);
        auto mario = std::make_unique<model::Mario>(model::Vector2{64.0f, groundY});
        playerPtr = mario.get();
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

    // The goal castle is painted into the grid from its 21-tile sheet (see
    // TileMap::CastleSymbols), row-major over a 5x5 silhouette standing on the ground:
    // the upper two rows are the 3-wide tower, the lower three the 5-wide base, the
    // centre-bottom pair is the door, and the two outer cells of the tower rows stay
    // air. The paint is deterministic, so re-running resetLevel (enter/death) is
    // idempotent.
    const std::size_t groundRowTop =
        rows - 1 - static_cast<std::size_t>(groundTop / static_cast<float>(tileHeight));
    const std::size_t castleCol = baseColumns + CastleOffsetTiles;
    std::size_t castleIndex = 0;
    for (std::size_t silhouetteRow = 0; silhouetteRow < 5; ++silhouetteRow) {
        for (std::size_t silhouetteColumn = 0; silhouetteColumn < 5; ++silhouetteColumn) {
            if (silhouetteRow < 2 && (silhouetteColumn == 0 || silhouetteColumn == 4)) {
                continue;
            }
            map.setTile(groundRowTop + 5 - silhouetteRow, castleCol + silhouetteColumn,
                        model::TileMap::CastleSymbols[castleIndex++]);
        }
    }

    auto flag = std::make_unique<model::FlagPole>(
        model::Vector2{static_cast<float>((baseColumns + PoleOffsetTiles) * tileWidth),
                       groundTop - poleHeight},
        model::Vector2{8.0f, poleHeight});
    flagPolePtr = flag.get();
    entities.push_back(std::move(flag));

    // Every character obeys the current world's physics (gravity/fall/drag, swim).
    const model::World& world = model::WorldSet::forType(worldType);
    for (const auto& e : entities) {
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            character->setWorld(world);
        }
    }
}

LevelScene::Event LevelScene::update(float deltaTime) {
    // While the owner's clear cinematic runs, the world is frozen: no timer, no input,
    // no physics. The cinematic drives the player directly.
    if (cinematicActive) {
        return Event::None;
    }

    // SMB timer: one tick per second. Running out of time is a death.
    if (playerPtr && !playerPtr->isDying()) {
        timer.update(deltaTime);
        if (timer.isExpired()) {
            playerPtr->die(true);
        }
    }

    std::vector<model::Entity*> activeEntities;
    for (auto& e : entities) {
        if (!e->isActive) continue;

        // Input gathering is delegated polymorphically: only the player reacts (input is
        // a Character capability — static world objects have no input). It runs BEFORE
        // entity->update() so gravity & integration see the correct player-intended
        // velocity, not stale values.
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            character->handleInput(deltaTime);
        }

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
        if (!mapLoaded && ++failFrame % 10 == 0 && e.get() == playerPtr) {
            trace("fail pos=" + std::to_string(pos.x) + "," + std::to_string(pos.y)
                  + " g=" + std::to_string(playerPtr->isGrounded)
                  + " vy=" + std::to_string(playerPtr->getVelocity().y));
        }

        // Bodies that finished their (non-animated) death are gone for good, e.g.
        // squished Goombas after their despawn timer. Only Characters have life state;
        // static world objects are always alive and skip these checks.
        auto* character = dynamic_cast<model::Character*>(e.get());
        if (character && !character->isAlive() && !character->isDying()) {
            e->isActive = false;
            continue;
        }

        // Dying bodies fall through the world; once past the bottom they are removed.
        if (character && character->isDying()) {
            if (pos.y > mapHeight) {
                if (e.get() == playerPtr) {
                    playerFinishedDeathFall = true;
                }
                e->isActive = false;
            }
            continue;
        }

        if (e.get() == playerPtr) {
            // The player cannot leave the map; a fall past the bottom is a pit death.
            // The FEET decide: the y-clamp below pins the player to mapHeight - size.y,
            // so checking the top (pos.y) can never be exceeded and the death would
            // never trigger.
            if (pos.y + sz.y >= mapHeight) {
                playerPtr->die(false); // no bounce: the body just keeps dropping
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

    // The player's death fall is over: the owner either ends the run or restarts.
    if (playerFinishedDeathFall) {
        return Event::RunEnded;
    }

    // Pipe entry: holding Down while standing on a pipe's cap and a portal is bound to
    // that pipe's column teleports the player to the portal's area.
    if (playerPtr) {
        if (const model::Portal* portal =
                portals.findEntryPortal(*playerPtr, level, currentArea, entities)) {
            teleportToPortal(*portal);
        }
    }

    // Flagpole touch: report the event so the owner starts the scripted clear play.
    if (flagPolePtr && flagPolePtr->isTouched() && playerPtr && !playerPtr->isDying()) {
        return Event::ClearTriggered;
    }

    return Event::None;
}

void LevelScene::render(sf::RenderTarget& window) {
    // Camera: follows the player horizontally, but never pans past the map fringes; it is
    // fixed vertically. The view keeps the fixed viewport set by AppEngine.
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float halfWidth = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
    float cameraX = halfWidth;
    if (playerPtr) {
        cameraX = playerPtr->getPosition().x + playerPtr->getSize().x / 2.0f;
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
        const view::RenderContext ctx{worldType};
        for (const auto& e : entities) {
            if (e->isActive) {
                entityRenderers->render(window, *e, ctx);
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

    // Restore the fixed (non-scrolling) view so the owner's screen-space HUD stays put.
    window.setView(baseView);
}

model::Player* LevelScene::player() const {
    return playerPtr;
}

model::FlagPole* LevelScene::flagPole() const {
    return flagPolePtr;
}

int LevelScene::getRemainingTime() const {
    return timer.getRemainingSeconds();
}

void LevelScene::pauseTimer() {
    timer.pause();
}

void LevelScene::setCinematicActive(bool active) {
    cinematicActive = active;
}

void LevelScene::toggleHitboxes() {
    showHitboxes = !showHitboxes;
}

float LevelScene::castleDoorX() const {
    return static_cast<float>(
        (map.getColumns() - LevelPaddingTiles + CastleOffsetTiles) * model::TileMap::TileWidth);
}

}