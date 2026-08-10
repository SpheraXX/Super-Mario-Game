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
#include "Model/Level/Pipe.h"
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
#include "View/Level/PipeRenderer.h"
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

// Duration of the pole-slide segment before Mario walks on.
constexpr float SlideDuration = 0.45f;
// Auto-walk speed on the flat completion zone after the slide.
constexpr float CinematicWalkSpeed = 220.0f;

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
        level.loadFromFile(game.getCurrentMapPath());
        loadArea(0);
    } catch (const std::exception& error) {
        std::cerr << "PlayState: failed to load level assets: " << error.what() << '\n';
        mapLoaded = false;
    }

    timer.reset(TimerStartSeconds);
    levelComplete = false;

    // Publish the level's metadata where the HUD and the completion flow read it.
    game.setLevelName(level.getLevelName());
    game.setNextMapPath(level.getNextMapPath());

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
    entityRenderers->registerRenderer<model::Pipe, view::PipeRenderer>();
    hudRenderer = std::make_unique<view::HudRenderer>();

    // Initialize collision manager
    collisionManager = std::make_unique<model::CollisionManager>(&map);

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
}

// Instantiate the given area: copy its grid into the working map, rebuild the themed
// renderer, append the completion zone on the FINAL area only, then spawn the area.
void PlayState::loadArea(std::size_t areaIndex) {
    currentArea = areaIndex;
    inertPipeColumns.clear();  // every visit to an area reactivates all its pipes
    worldType = level.areaWorld(areaIndex);
    map = level.areaMap(areaIndex);
    if (currentArea == level.areaCount() - 1) {
        map.padRight(LevelPaddingTiles);
    }
    renderer = std::make_unique<view::TileMapRenderer>("assets/blocks.png", worldType);
    mapLoaded = true;
    resetLevel();
}

void PlayState::teleportToPortal(const model::Portal& portal) {
    if (portal.destinationArea >= level.areaCount()) {
        return;
    }

    // Rebuild the destination area and its entities, then place Mario either on the
    // cap of the destination pipe (if the arrival column has one) or on the ground.
    // The camera, HUD and timer all keep their state.
    loadArea(portal.destinationArea);
    inertPipeColumns.push_back(portal.destinationColumn);  // one-way: no re-entry here
    if (!player) {
        return;
    }
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const float groundTop = groundTopAt(map, portal.destinationColumn);
    float landY = groundTop - player->getSize().y;
    for (const auto& e : entities) {
        auto* pipe = dynamic_cast<model::Pipe*>(e.get());
        if (pipe && pipe->getSourceColumn() == portal.destinationColumn) {
            landY = pipe->getPosition().y - player->getSize().y;
            break;
        }
    }
    player->setPosition({
        static_cast<float>(portal.destinationColumn * tileWidth),
        landY});
    player->setVelocity({0.0f, 0.0f});
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
    castle = nullptr;

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

    // Pipes: contiguous vertical runs of 'P' on one column become a single Pipe whose
    // box covers the whole run (cap on the top cell, plain body below). 'P' marks the
    // LEFT column of a 2-tile-wide pipe: the cell to its right must stay empty ('.'/'-'),
    // so the pipe spans two tiles while the map only encodes its left column. If the
    // right-hand cells are occupied the run falls back to a 1-wide pipe to stay playable.
    // The column is what links the entity to its level portal, if any.
    for (std::size_t column = 0; column < columns; ++column) {
        std::size_t runStart = 0;
        while (runStart < rows) {
            while (runStart < rows && map.getTile(runStart, column) != 'P') {
                ++runStart;
            }
            if (runStart >= rows) {
                break;
            }
            std::size_t runEnd = runStart;
            while (runEnd + 1 < rows && map.getTile(runEnd + 1, column) == 'P') {
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
    castle = static_cast<model::Castle*>(entities.back().get());

    // Every character obeys the current world's physics (gravity/fall/drag, swim).
    const model::World& world = model::WorldSet::forType(worldType);
    for (const auto& e : entities) {
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            character->setWorld(world);
        }
    }

    // A fresh level starts outside the clear sequence.
    clearPhase = ClearPhase::None;
    completionOverlayPushed = false;
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
    // Once the level is complete the game is frozen behind the completion overlay:
    // no timer, no input, no physics.
    if (levelComplete) {
        return;
    }

    // After the flagpole is touched a short scripted clear play keeps updating the
    // tableau (pole slide, walk to the castle) until the overlay is pushed.
    if (clearPhase != ClearPhase::None) {
        updateClearSequence(deltaTime);
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

    // Pipe entry: holding Down while standing on a pipe's cap and a portal is bound to
    // that pipe's column teleports the player to the portal's area.
    if (player && player->getInputDown() && !player->isDying() && !levelComplete &&
        clearPhase == ClearPhase::None) {
        for (const auto& e : entities) {
            auto* pipe = dynamic_cast<model::Pipe*>(e.get());
            if (!pipe || !pipe->isActive) continue;
            const model::Portal* portal = nullptr;
            for (const auto& p : level.portals(currentArea)) {
                if (p.sourceColumn == pipe->getSourceColumn()) {
                    portal = &p;
                    break;
                }
            }
            if (!portal) continue;

            // One-way pipes: the pipe the player arrived through is inert for this visit.
            if (std::find(inertPipeColumns.begin(), inertPipeColumns.end(),
                          pipe->getSourceColumn()) != inertPipeColumns.end()) {
                continue;
            }

            const model::Vector2& pPos = player->getPosition();
            const float feetY = pPos.y + player->getSize().y;
            const float onTop = std::abs(feetY - pipe->getPosition().y);
            // Entry needs the player's feet resting on the cap and a real footprint
            // overlap with it (slightly forgiving at the very edge).
            const bool overlapsCap = pPos.x + player->getSize().x > pipe->getPosition().x + 2.0f &&
                                     pPos.x < pipe->getPosition().x + pipe->getSize().x - 2.0f;
            if (player->isGrounded && onTop < 8.0f && overlapsCap) {
                teleportToPortal(*portal);
                break;
            }
        }
    }

    // Flagpole touch: award the clear bonus (flag height + time remaining) and start the
    // scripted clear play. update() keeps running the sequence (feeding the frozen
    // tableau) until the completion overlay is finally pushed.
    if (flagPole && flagPole->isTouched() && player && !player->isDying()) {
        beginLevelClear();
    }

    // HUD snapshot for the next frame.
    auto& game = model::GameManager::instance();
    hudData.score = game.getScore();
    hudData.coins = game.getCoins();
    hudData.levelName = game.getLevelName();
    hudData.time = timer.getRemainingSeconds();
}

void PlayState::beginLevelClear() {
    if (!flagPole || !player) {
        finishLevelClear();
        return;
    }

    // Award the clear bonus immediately (flag + time remaining), like the timer path.
    timer.pause();
    const int timeBonus = timer.getRemainingSeconds() * TimeBonusPerSecond;
    model::GameManager::instance().addScore(FlagBonus + timeBonus);
    model::GameManager::instance().setLevelClearBonus(FlagBonus + timeBonus);
    trace("clearBonus bonus=" + std::to_string(FlagBonus + timeBonus));

    // Remember the geometry the sequence animates against.
    const model::Vector2 poleTop = flagPole->getPosition();
    poleGroundY = poleTop.y + flagPole->getSize().y;
    poleSlideStartY = player->getPosition().y;
    castleEntryX = (castle) ? castle->getPosition().x : 0.0f;

    poleElapsed = 0.0f;
    clearPhase = ClearPhase::SlideToPole;
}

void PlayState::updateClearSequence(float deltaTime) {
    if (!player || !flagPole) {
        finishLevelClear();
        return;
    }

    switch (clearPhase) {
        case ClearPhase::SlideToPole: {
            poleElapsed += deltaTime;
            const float progress = std::clamp(poleElapsed / SlideDuration, 0.0f, 1.0f);
            flagPole->setSlideProgress(progress);

            // Mario hugs the pole and slides with the pennant from where he touched it
            // down to the ground. The horizontal position is fixed to the pole's column
            // (centred), the vertical lerps to the flat completion-zone ground.
            const float poleX = static_cast<float>(flagPole->getPosition().x);
            const float targetY = poleGroundY - player->getSize().y;
            player->setPosition({
                poleX + (flagPole->getSize().x - player->getSize().x) / 2.0f,
                poleSlideStartY + (targetY - poleSlideStartY) * progress});
            player->setVelocity({0.0f, 0.0f});
            player->isGrounded = true;
            player->setFacingRight(true);

            if (progress >= 1.0f) {
                clearPhase = ClearPhase::WalkToCastle;
            }
            break;
        }
        case ClearPhase::WalkToCastle: {
            // Auto-walk rightwards on the flat completion zone until the castle door.
            const float marioLeft = player->getPosition().x;
            player->setVelocity({CinematicWalkSpeed, 0.0f});
            player->setFacingRight(true);
            player->update(deltaTime);
            if (marioLeft + player->getSize().x / 2.0f >= castleEntryX) {
                player->setVelocity({0.0f, 0.0f});
                clearPhase = ClearPhase::ReachedCastle;
            }
            break;
        }
        case ClearPhase::ReachedCastle:
        case ClearPhase::None:
            finishLevelClear();
            break;
    }
}

void PlayState::finishLevelClear() {
    if (completionOverlayPushed) {
        return;
    }
    completionOverlayPushed = true;
    levelComplete = true;
    manager->pushState(std::make_unique<LevelCompleteState>());
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

    // Restore the fixed (non-scrolling) view so HUD text stays on the screen.
    window.setView(baseView);

    if (hudRenderer) {
        hudRenderer->render(window, hudData);
    }
}

}
