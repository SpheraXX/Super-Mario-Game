#include "Controller/LevelScene.h"

#include "Controller/AppEngine.h"
#include "Model/Core/LogManager.h"
#include "Model/Save/SaveData.h"
#include "Model/Block/BrickBlock.h"
#include "Model/Block/BrickShard.h"
#include "Model/Block/CoinBlock.h"
#include "Model/Character.h"
#include "Model/Core/GameManager.h"
#include "Model/Core/VerticalSlide.h"
#include "Model/Enemy/Bowser.h"
#include "Model/Enemy/CheepCheep.h"
#include "Model/Enemy/EnemyFactory.h"
#include "Model/Enemy/HammerBro.h"
#include "Model/Enemy/Lakitu.h"
#include "Model/Enemy/PiranhaPlant.h"
#include "Model/Enemy/Spiny.h"
#include "Model/Item/Coin.h"
#include "Model/Item/MapCoin.h"
#include "Model/Item/FireFlower.h"
#include "Model/Item/Mushroom.h"
#include "Model/Item/Starman.h"
#include "Model/Projectile/Fireball.h"
#include "Model/Projectile/Hammer.h"
#include "Model/Projectile/MarioFireball.h"
#include "Model/Projectile/SpinyEgg.h"
#include "Model/Enemy/Goomba.h"
#include "Model/Enemy/Koopa.h"
#include "Model/Level/ChainTrigger.h"
#include "Model/Level/FirebarBall.h"
#include "Model/Level/FlagPole.h"
#include "Model/Level/LavaBubble.h"
#include "Model/Level/LevelGoal.h"
#include "Model/Level/Pipe.h"
#include "Model/Level/Slider.h"
#include "Model/NPC/MushroomRetainer.h"
#include "Model/Player/Luigi.h"
#include "Model/Player/Mario.h"
#include "Model/Player/Player.h"
#include "Model/World/WorldSet.h"
#include "View/Base/RenderContext.h"
#include "View/Block/BrickBlockRenderer.h"
#include "View/Block/BrickShardRenderer.h"
#include "View/Block/CoinBlockRenderer.h"
#include "View/Base/AtlasFrameRenderer.h"
#include "View/Base/MiscFrameRenderer.h"
#include "View/Enemy/HammerRenderer.h"
#include "View/Level/ChainTriggerRenderer.h"
#include "View/Level/FirebarBallRenderer.h"
#include "View/Level/FlagPoleRenderer.h"
#include "View/Enemy/FireballRenderer.h"
#include "View/Item/ItemFrameRenderer.h"
#include "View/Item/MapCoinRenderer.h"
#include "View/Enemy/GoombaRenderer.h"
#include "View/Enemy/KoopaRenderer.h"
#include "View/Level/LevelGoalRenderer.h"
#include "View/Level/SliderRenderer.h"
#include "View/Player/PlayerRenderer.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

namespace controller {

namespace {
constexpr float TimerStartSeconds = 400.0f;

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
    entityRenderers->registerRenderer<model::BrickShard, view::BrickShardRenderer>();
    entityRenderers->registerRenderer<model::LevelGoal, view::LevelGoalRenderer>();
    entityRenderers->registerRenderer<model::Slider, view::SliderRenderer>();

    // Pipes are NOT registered: they are terrain, drawn per cell by the tile renderer
    // ('P'/'Q'/'p'/'q' cells). A warp pipe spawns a model::Pipe entity for the portal
    // linkage, but rendering it on top with a different sprite set made warp pipes look
    // unlike regular pipes — the entity is intentionally invisible.

    // Items are drawn from their own sprite sheet; the frame rects are named in
    // View/Item/ItemAtlas.h.
    entityRenderers->registerRenderer<model::Mushroom,
                                      view::ItemFrameRenderer<model::Mushroom>>(view::atlas::Mushroom);
    entityRenderers->registerRenderer<model::FireFlower,
                                      view::ItemFrameRenderer<model::FireFlower>>(view::atlas::FireFlower);
    entityRenderers->registerRenderer<model::Starman,
                                      view::ItemFrameRenderer<model::Starman>>(view::atlas::Starman);
    // The coin comes off the main Mario sheet instead, which needs its backdrop keyed out.
    entityRenderers->registerRenderer<model::Coin, view::ItemFrameRenderer<model::Coin>>(
        view::atlas::Coin, view::atlas::MarioAssetSheet, view::atlas::MarioAssetColorKey);
    // The placed coin needs its own renderer rather than the fixed-frame template, because
    // it cycles. The registry dispatches on exact typeid, so it needs its own entry even
    // though it shares both the sheet and the artwork with the block flourish above.
    entityRenderers->registerRenderer<model::MapCoin, view::MapCoinRenderer>();

    // Everything below has a single pose and shares the generic atlas renderer; the frames
    // themselves are named in View/Enemy/EnemyAtlas.h.
    entityRenderers->registerRenderer<model::HammerBro,
                                      view::AtlasFrameRenderer<model::HammerBro>>(view::atlas::HammerBro);
    entityRenderers->registerRenderer<model::Lakitu,
                                      view::AtlasFrameRenderer<model::Lakitu>>(view::atlas::Lakitu);
    entityRenderers->registerRenderer<model::Spiny,
                                      view::AtlasFrameRenderer<model::Spiny>>(view::atlas::Spiny);
    entityRenderers->registerRenderer<model::Bowser,
                                      view::AtlasFrameRenderer<model::Bowser>>(view::atlas::Bowser);
    entityRenderers->registerRenderer<model::PiranhaPlant,
                                      view::AtlasFrameRenderer<model::PiranhaPlant>>(view::atlas::PiranhaPlant);
    // The hammer spins through four poses off misc.png, so it needs a renderer of its own
    // rather than the fixed-frame template the rest of this block uses.
    entityRenderers->registerRenderer<model::Hammer, view::HammerRenderer>();
    entityRenderers->registerRenderer<model::SpinyEgg,
                                      view::AtlasFrameRenderer<model::SpinyEgg>>(view::atlas::SpinyEgg);
    entityRenderers->registerRenderer<model::Fireball,
                                      view::AtlasFrameRenderer<model::Fireball>>(view::atlas::BowserFire);
    // Mario's fireball is its own animated ball (4 rolling frames), unlike Bowser's flat
    // breath above, so it needs its own renderer.
    entityRenderers->registerRenderer<model::MarioFireball, view::FireballRenderer>();

    // Castle furniture and the water level's fish, all off misc.png (see MiscAtlas.h).
    // The firebar's flame picks its pose from its own sweep angle, so it needs a renderer;
    // the rest are single-frame and share the misc-sheet template.
    entityRenderers->registerRenderer<model::FirebarBall, view::FirebarBallRenderer>();
    entityRenderers->registerRenderer<model::LavaBubble,
                                      view::MiscFrameRenderer<model::LavaBubble>>(
        view::atlas::LavaBubble);
    entityRenderers->registerRenderer<model::CheepCheep,
                                      view::MiscFrameRenderer<model::CheepCheep>>(
        view::atlas::CheepCheep[0]);
    entityRenderers->registerRenderer<model::MushroomRetainer,
                                      view::MiscFrameRenderer<model::MushroomRetainer>>(
        view::atlas::MushroomRetainer);
    entityRenderers->registerRenderer<model::ChainTrigger, view::ChainTriggerRenderer>();
    entityRenderers->registerRenderer<model::FlagPole, view::FlagPoleRenderer>();
}

bool LevelScene::loadLevel(const model::LevelSaveData* levelSave, const model::PlayerSaveData* playerSave) {
    auto& game = model::GameManager::instance();
    try {
        level.loadFromFile(game.getCurrentMapPath());
        std::size_t targetArea = 0;
        if (levelSave && levelSave->currentArea < level.areaCount()) {
            targetArea = levelSave->currentArea;
        }
        loadArea(targetArea, /* keepPlayer */ false, levelSave, playerSave);
    } catch (const std::exception& error) {
        model::LogManager::instance().error(std::string("Failed to load level: ") + error.what());
        mapLoaded = false;
        return false;
    }

    // Publish the level's metadata where the HUD and the completion flow read it.
    game.setLevelName(level.getLevelName());
    game.setNextMapPath(level.getNextMapPath());

    if (levelSave) {
        timer.reset(levelSave->remainingTime);
    } else {
        timer.reset(TimerStartSeconds);
    }

    if (playerSave && playerPtr) {
        playerPtr->restoreState(*playerSave);
        armDormancy();
    }

    model::LogManager::instance().info("Level start: " + game.getLevelName());
    return true;
}

void LevelScene::setInputMapper(model::IInputMapper* mapper) {
    inputMapper = mapper;
}

// Instantiate the given area: copy its grid into the working map, rebuild the themed
// renderer, append the completion zone on the FINAL area only, then spawn the area.
void LevelScene::loadArea(std::size_t areaIndex, bool keepPlayer, const model::LevelSaveData* levelSave, const model::PlayerSaveData* playerSave) {
    currentArea = areaIndex;
    portals.clear();  // every visit to an area reactivates all its pipes
    worldType = level.areaWorld(areaIndex);
    map = level.areaMap(areaIndex);
    if (currentArea == level.areaCount() - 1) {
        map.padRight(LevelCompletion::LevelPaddingTiles);
    }

    // Apply removed / broken tiles if restoring this area
    if (levelSave && levelSave->currentArea == currentArea) {
        for (const auto& tile : levelSave->removedTiles) {
            if (tile.row < map.getRows() && tile.col < map.getColumns()) {
                map.setTile(tile.row, tile.col, '.');
            }
        }
    }

    renderer = std::make_unique<view::TileMapRenderer>("assets/blocks.png", worldType);
    mapLoaded = true;
    resetLevel(keepPlayer, levelSave, playerSave);
}

void LevelScene::teleportToPortal(const model::Portal& portal) {
    if (portal.destinationArea >= level.areaCount()) {
        return;
    }

    // Rebuild the destination area and its entities, then place Mario either on the
    // cap of the destination pipe (if the arrival column has one) or on the ground.
    // The camera, HUD and timer all keep their state; the player is kept so that
    // his size and power-ups survive the area change.
    loadArea(portal.destinationArea, /* keepPlayer */ true);
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

void LevelScene::beginPipeTransition(const model::Portal& portal) {
    if (!playerPtr) {
        return;
    }
    pendingPortal = portal;
    timer.pause();
    timerPausedByPipe = true;

    // Which axis to sink along depends on the pipe findEntryPortal matched, not the
    // portal itself (a Portal is just a column binding). The destination leg further
    // down in advancePipeTransition always rises vertically onto the arrival cap,
    // regardless of the source's orientation -- only entry has a horizontal variant.
    model::Pipe::Orientation orientation = model::Pipe::Orientation::Vertical;
    for (const auto& e : entities) {
        auto* pipe = dynamic_cast<model::Pipe*>(e.get());
        if (pipe && pipe->getSourceColumn() == portal.sourceColumn) {
            orientation = pipe->getOrientation();
            break;
        }
    }

    if (orientation == model::Pipe::Orientation::Horizontal) {
        // Sink rightward: findEntryPortal proved the player's right edge rests on the
        // pipe's left face, so one body width further hides him fully inside it.
        const float rightEdge = playerPtr->getPosition().x + playerPtr->getSize().x;
        playerPtr->beginPipeSlide(rightEdge + playerPtr->getSize().x,
                                  model::VerticalSlide::Axis::Horizontal);
    } else {
        // Sink the whole body below the pipe's mouth: findEntryPortal proved the feet
        // rest on the cap, so one body height down hides the player fully behind the
        // pipe shaft.
        const float capTop = playerPtr->getPosition().y + playerPtr->getSize().y;
        playerPtr->beginPipeSlide(capTop + playerPtr->getSize().y);
    }
    pipePhase = PipePhase::SlideIn;
}

void LevelScene::advancePipeTransition(float deltaTime) {
    if (!playerPtr) {
        return;
    }

    // A death mid-slide (debug key) aborts the travel and resumes the world.
    if (!playerPtr->isAlive() || playerPtr->isDying()) {
        playerPtr->endPipeSlide();
        if (timerPausedByPipe) {
            timer.resume();
            timerPausedByPipe = false;
        }
        pipePhase = PipePhase::None;
        return;
    }

    if (pipePhase == PipePhase::SlideIn) {
        if (playerPtr->advancePipeSlide(deltaTime)) {
            return;  // still sinking
        }
        // Fully inside the source pipe: travel, then set up the slide-out. The body is
        // placed one height below the destination cap (hidden inside the pipe) and rises
        // to rest on it; the snap is invisible because it draws behind the terrain.
        teleportToPortal(pendingPortal);
        if (!playerPtr) {
            return;
        }
        const float sink = playerPtr->getSize().y;
        playerPtr->setPosition(
            {playerPtr->getPosition().x, playerPtr->getPosition().y + sink});
        playerPtr->beginPipeSlide(playerPtr->getPosition().y - sink);
        pipePhase = PipePhase::SlideOut;
        return;
    }

    // SlideOut: the rise ends with Mario resting on the destination cap.
    if (playerPtr->advancePipeSlide(deltaTime)) {
        return;  // still rising
    }
    playerPtr->endPipeSlide();
    if (timerPausedByPipe) {
        timer.resume();
        timerPausedByPipe = false;
    }
    pipePhase = PipePhase::None;
}

// (Re)build the entity list from scratch: the map file drives what spawns where.
// 'M' = Mario, 'C' = CoinBlock, '#'/'B' = BrickBlock. Enemy markers are the digits 0-9
// (EnemyFactory ids), placed in the cell directly above the ground: every enemy's feet
// rest on that marker cell's bottom edge, so a body taller than one tile is dropped by
// its overhang (see the digit loop below). Digits are stripped to empty tiles at load,
// so a marker never doubles as terrain. Called on enter and after every death (the
// whole level restarts). With keepPlayer=true the current player survives the rebuild,
// so his size and power-ups carry over when a warp pipe changes area.
void LevelScene::restartLevel() {
    // A death always restarts the whole run from the first area, whatever area the body
    // fell in; loadArea(0) rebuilds area 0 with a fresh Mario (keepPlayer=false default).
    loadArea(0);
}

void LevelScene::resetLevel(bool keepPlayer, const model::LevelSaveData* levelSave, const model::PlayerSaveData* playerSave) {
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const std::size_t tileHeight = model::TileMap::TileHeight;
    const std::size_t rows = map.getRows();
    const std::size_t columns = map.getColumns();

    // An area change keeps Mario: release his unique_ptr from the list before the clear
    // destroys it, and re-add it below (the teleport re-sets his position afterwards).
    std::unique_ptr<model::Entity> keptPlayer;
    if (keepPlayer && playerPtr) {
        for (auto& entity : entities) {
            if (entity.get() == playerPtr) {
                keptPlayer = std::move(entity);
                break;
            }
        }
    }

    entities.clear();
    if (!keptPlayer) {
        playerPtr = nullptr;  // full restart: a fresh player spawns below
    }
    goalPtr = nullptr;

    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            const char symbol = map.getTile(row, column);
            const model::Vector2 position{static_cast<float>(column * tileWidth),
                                          static_cast<float>((rows - 1 - row) * tileHeight)};
            const model::Vector2 size{static_cast<float>(tileWidth),
                                      static_cast<float>(tileHeight)};

            switch (symbol) {
                case 'M':
                    if (!playerPtr) {
                        std::unique_ptr<model::Player> player;
                        if (playerSave && playerSave->isLuigi) {
                            player = std::make_unique<model::Luigi>(position);
                        } else {
                            player = std::make_unique<model::Mario>(position);
                        }
                        playerPtr = player.get();
                        entities.push_back(std::move(player));
                    }
                    break;
                case 'C': {
                    auto coinBlock = std::make_unique<model::CoinBlock>(position, size);
                    if (levelSave && levelSave->currentArea == currentArea) {
                        for (const auto& cbData : levelSave->coinBlocks) {
                            if (std::abs(cbData.posX - position.x) < 1.0f && std::abs(cbData.posY - position.y) < 1.0f) {
                                if (cbData.opened) {
                                    coinBlock->setCoinAvailable(false);
                                }
                                break;
                            }
                        }
                    }
                    entities.push_back(std::move(coinBlock));
                    break;
                }
                case model::TileMap::CoinSymbol:
                    entities.push_back(std::make_unique<model::MapCoin>(position));
                    break;
                case '#':
                case 'B':
                    entities.push_back(std::make_unique<model::BrickBlock>(position, size));
                    break;
                case model::TileMap::HorizontalPipeSymbol: {
                    // Unlike a standing pipe, this footprint has no per-cell terrain
                    // equivalent (see TileMap::HorizontalPipeSymbol) -- the entity is its
                    // only source of collision, so it is spawned unconditionally, portal
                    // or not, exactly as the comment on Pipe::Orientation promises.
                    // 'position' is this cell's own top-left, which is also the box's
                    // top-left: the anchor is the topmost-on-screen, leftmost cell of the
                    // 4x2 footprint (see TileMap::HorizontalPipeSymbol).
                    const model::Vector2 pipeSize{4.0f * size.x, 2.0f * size.y};
                    entities.push_back(std::make_unique<model::Pipe>(
                        position, pipeSize, column, model::Pipe::Orientation::Horizontal));
                    break;
                }
                case model::TileMap::FirebarSymbol: {
                    // The marker cell is both the mount (solid terrain, drawn by the tile
                    // renderer) and the pivot, so the bar turns about the cell's CENTRE.
                    // The bar is a line of independent flames rather than one entity —
                    // see Model/Level/FirebarBall.h for why a rotating arm cannot be one.
                    constexpr int Links = 4;
                    constexpr float LinkStep = 8.0f;
                    constexpr float SpinSpeed = 2.0f;  // radians/second
                    const model::Vector2 pivot{position.x + size.x * 0.5f,
                                               position.y + size.y * 0.5f};
                    for (int link = 1; link <= Links; ++link) {
                        entities.push_back(std::make_unique<model::FirebarBall>(
                            pivot, LinkStep * static_cast<float>(link), SpinSpeed, 0.0f));
                    }
                    break;
                }
                case model::TileMap::LavaBubbleSymbol: {
                    constexpr float RiseHeight = 5.0f * 16.0f;  // five tiles out of the pool
                    constexpr float LeapSeconds = 1.6f;
                    constexpr float RestSeconds = 1.0f;
                    entities.push_back(std::make_unique<model::LavaBubble>(
                        position, RiseHeight, LeapSeconds, RestSeconds));
                    break;
                }
                case model::TileMap::ChainTriggerSymbol:
                    entities.push_back(std::make_unique<model::ChainTrigger>(position, size));
                    break;
                case model::TileMap::RetainerSymbol:
                    entities.push_back(std::make_unique<model::MushroomRetainer>(position));
                    break;
                case model::TileMap::GoalSymbol: {
                    // A trigger several cells tall so a jumping player cannot skip over
                    // it; the marked cell is the BOTTOM of the column, matching where
                    // the author stood it (the same anchor a flagpole used to have).
                    constexpr float GoalHeightTiles = 4.0f;
                    const model::Vector2 goalSize{size.x, size.y * GoalHeightTiles};
                    const model::Vector2 goalPos{
                        position.x, position.y - size.y * (GoalHeightTiles - 1.0f)};
                    auto goal = std::make_unique<model::LevelGoal>(goalPos, goalSize);
                    goalPtr = goal.get();
                    entities.push_back(std::move(goal));
                    break;
                }
                default:
                    break;
            }
        }
    }

    // Pipes are solid TERRAIN (TileMap::isSolidTile), drawn per cell by the tile renderer:
    // 'P'/'Q' are the mouth's left/right cells and 'p'/'q' the shaft below. That is what
    // makes enemies collide with them — the entity pass only resolves the player against
    // solid entities, so an entity-only pipe is invisible to everything else.
    //
    // A Pipe ENTITY is still spawned, but only for a column that carries a warp portal:
    // PortalSystem matches a Portal to a Pipe by its source column, so the entity is the
    // linkage that makes "hold Down to enter" work. Ordinary scenery pipes need no entity.
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
            // The right column is explicit in the map ('Q'/'q'); an empty cell is also
            // accepted so older maps that only encoded the left column still work.
            bool wide = column + 1 < columns;
            if (wide) {
                for (std::size_t row = runStart; row <= runEnd && wide; ++row) {
                    const char rightCell = map.getTile(row, column + 1);
                    wide = (rightCell == 'Q' || rightCell == 'q'
                            || rightCell == '.' || rightCell == '-');
                }
            }
            const float pipeWidth =
                wide ? 2.0f * static_cast<float>(tileWidth) : static_cast<float>(tileWidth);
            bool hasPortal = false;
            for (const auto& portal : level.portals(currentArea)) {
                if (portal.sourceColumn == column) {
                    hasPortal = true;
                    break;
                }
            }
            if (hasPortal) {
                auto pipe = std::make_unique<model::Pipe>(
                    model::Vector2{static_cast<float>(column * tileWidth), pipeTop},
                    model::Vector2{pipeWidth, pipeHeight}, column);
                entities.push_back(std::move(pipe));
            }
            runStart = runEnd + 1;
        }
    }

    // Sliders: a 2-cell '=' run gives the platform its fixed shape (its art is a constant
    // 32x8, unlike a pipe's author-chosen height), and the run's leftmost column binds it
    // to a '; slider=' token for its motion — the same scheme the pipe loop above uses to
    // bind a Pipe to its Portal.
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t column = 0; column < columns; ++column) {
            if (map.getTile(row, column) != model::TileMap::SliderSymbol) continue;
            if (column > 0 && map.getTile(row, column - 1) == model::TileMap::SliderSymbol) {
                continue;  // the run's second cell; only its leftmost cell starts a slider
            }
            for (const auto& spec : level.sliders(currentArea)) {
                if (spec.sourceColumn != column) continue;
                const model::Vector2 origin{static_cast<float>(column * tileWidth),
                                            static_cast<float>((rows - 1 - row) * tileHeight)};
                const auto axis = spec.axis == model::SliderAxis::Horizontal
                    ? model::Slider::Axis::Horizontal
                    : model::Slider::Axis::Vertical;
                entities.push_back(std::make_unique<model::Slider>(
                    origin,
                    model::Vector2{2.0f * static_cast<float>(tileWidth),
                                  static_cast<float>(tileHeight)},
                    axis, spec.travelDistance, spec.speed));
                break;
            }
        }
    }

    // Fallback: if the map has no 'M', keep the game playable with a fixed spawn.
    if (!playerPtr) {
        const float groundY = static_cast<float>((rows - 2) * tileHeight - tileHeight);
        std::unique_ptr<model::Player> player;
        if (playerSave && playerSave->isLuigi) {
            player = std::make_unique<model::Luigi>(
                model::Vector2{static_cast<float>(2 * tileWidth), groundY});
        } else {
            player = std::make_unique<model::Mario>(
                model::Vector2{static_cast<float>(2 * tileWidth), groundY});
        }
        playerPtr = player.get();
        entities.push_back(std::move(player));
    }

    // An area change keeps Mario: put him back ahead of pipes and enemies (matching the
    // original spawn order) and refresh the pointer that owns the kept entity.
    if (keptPlayer) {
        playerPtr = static_cast<model::Player*>(keptPlayer.get());
        entities.push_back(std::move(keptPlayer));
    } else {
        model::LogManager::instance().info("Player spawn");
    }

    // Enemies & Items: If restored from snapshot, spawn exact living enemies and items
    if (levelSave && levelSave->hasEntitiesSnapshot && levelSave->currentArea == currentArea) {
        for (const auto& e : levelSave->enemies) {
            std::unique_ptr<model::Enemy> enemy;
            const model::Vector2 origin{e.posX, e.posY};
            if (e.type == "Goomba") {
                enemy = std::make_unique<model::Goomba>(origin);
            } else if (e.type == "Koopa") {
                auto koopa = std::make_unique<model::Koopa>(origin, e.isWinged);
                if (e.state == "ShellSpinning") {
                    koopa->setState(model::KoopaState::ShellSpinning);
                } else if (e.state == "ShellIdle") {
                    koopa->setState(model::KoopaState::ShellIdle);
                } else {
                    koopa->setState(model::KoopaState::Walking);
                }
                enemy = std::move(koopa);
            } else if (e.type == "HammerBro") {
                enemy = std::make_unique<model::HammerBro>(origin);
            } else if (e.type == "Lakitu") {
                enemy = std::make_unique<model::Lakitu>(origin);
            } else if (e.type == "Spiny") {
                enemy = std::make_unique<model::Spiny>(origin);
            } else if (e.type == "Bowser") {
                enemy = std::make_unique<model::Bowser>(origin);
            } else if (e.type == "PiranhaPlant") {
                enemy = std::make_unique<model::PiranhaPlant>(origin);
            }
            if (enemy) {
                enemy->setPosition({e.posX, e.posY});
                enemy->setVelocity({e.velX, e.velY});
                enemy->setFacingRight(e.facingRight);
                enemy->setDirection(e.direction);
                enemy->isDormant = e.isDormant;
                enemy->setMap(&map);
                entities.push_back(std::move(enemy));
            }
        }

        for (const auto& it : levelSave->items) {
            std::unique_ptr<model::Item> item;
            const model::Vector2 origin{it.posX, it.posY};
            if (it.type == "Mushroom") {
                item = std::make_unique<model::Mushroom>(origin, it.direction);
            } else if (it.type == "FireFlower") {
                item = std::make_unique<model::FireFlower>(origin);
            } else if (it.type == "Starman") {
                item = std::make_unique<model::Starman>(origin);
            } else if (it.type == "Coin") {
                item = std::make_unique<model::Coin>(origin);
            }
            if (item) {
                item->setPosition({it.posX, it.posY});
                item->setVelocity({it.velX, it.velY});
                item->setDirection(it.direction);
                item->isDormant = false;
                entities.push_back(std::move(item));
            }
        }
    } else {
        // Enemies placed as digit markers (EnemyFactory ids). These are stripped to empty
        // tiles at load, so they never double as terrain; the factory is the only place an
        // enemy is constructed for a level.
        for (const model::SpawnPoint& spawn : map.getSpawnPoints()) {
            const model::Vector2 origin = model::TileMap::tileOrigin(spawn.row, spawn.column);
            if (auto enemy = model::EnemyFactory::create(spawn.id, origin)) {
                enemy->setMap(&map);  // for ledge detection
                entities.push_back(std::move(enemy));
            }
        }
    }

    // Level completion zone, in the padded columns: flagpole, then the goal castle.
    if (currentArea == level.areaCount() - 1) {
        completion.build(map, entities);
    }
    // Every character obeys the current world's physics (gravity/fall/drag, swim), and
    // every entity gets this scene as its spawn channel (model::World).
    const model::WorldTheme& world = model::WorldSet::forType(worldType);
    for (const auto& e : entities) {
        e->setWorld(this);
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            character->setWorld(world);
        }
    }

    // Everything ahead of the camera starts asleep; the player is always awake.
    armDormancy();

    if (levelSave && levelSave->hasEntitiesSnapshot && levelSave->currentArea == currentArea) {
        std::size_t enemyIdx = 0;
        for (const auto& e : entities) {
            if (dynamic_cast<model::Enemy*>(e.get())) {
                if (enemyIdx < levelSave->enemies.size()) {
                    e->isDormant = levelSave->enemies[enemyIdx].isDormant;
                    enemyIdx++;
                }
            }
        }
    }
}

void LevelScene::captureLevelSaveData(model::LevelSaveData& outLevelSave) const {
    outLevelSave.currentArea = currentArea;
    outLevelSave.remainingTime = timer.getRemaining();

    // 1. Removed / broken tiles: compare working map with original map of this area
    outLevelSave.removedTiles.clear();
    const auto& originalMap = level.areaMap(currentArea);
    const std::size_t rows = map.getRows();
    const std::size_t cols = std::min(map.getColumns(), originalMap.getColumns());
    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < cols; ++c) {
            char orig = originalMap.getTile(r, c);
            char curr = map.getTile(r, c);
            if ((orig == '#' || orig == 'B' || orig == 'C') && curr == '.') {
                outLevelSave.removedTiles.push_back({r, c});
            }
        }
    }

    outLevelSave.coinBlocks.clear();
    outLevelSave.enemies.clear();
    outLevelSave.items.clear();

    for (const auto& e : entities) {
        if (!e) continue;

        // CoinBlock
        if (auto* cb = dynamic_cast<model::CoinBlock*>(e.get())) {
            model::BlockSaveData bsd;
            bsd.posX = cb->getPosition().x;
            bsd.posY = cb->getPosition().y;
            bsd.opened = cb->isOpened();
            outLevelSave.coinBlocks.push_back(bsd);
            continue;
        }

        // Enemy: save living ones
        if (auto* enemy = dynamic_cast<model::Enemy*>(e.get())) {
            if (enemy->isActive && enemy->isAlive() && !enemy->isDying()) {
                model::EnemySaveData esd;
                esd.posX = enemy->getPosition().x;
                esd.posY = enemy->getPosition().y;
                esd.velX = enemy->getVelocity().x;
                esd.velY = enemy->getVelocity().y;
                esd.isDormant = enemy->isDormant;
                esd.facingRight = enemy->isFacingRight();
                esd.direction = enemy->getDirection();

                if (auto* koopa = dynamic_cast<model::Koopa*>(enemy)) {
                    esd.type = "Koopa";
                    esd.isWinged = koopa->isWinged();
                    if (koopa->getState() == model::KoopaState::ShellSpinning) {
                        esd.state = "ShellSpinning";
                    } else if (koopa->getState() == model::KoopaState::ShellIdle) {
                        esd.state = "ShellIdle";
                    } else {
                        esd.state = "Walking";
                    }
                } else if (dynamic_cast<model::Goomba*>(enemy)) {
                    esd.type = "Goomba";
                } else if (dynamic_cast<model::HammerBro*>(enemy)) {
                    esd.type = "HammerBro";
                } else if (dynamic_cast<model::Lakitu*>(enemy)) {
                    esd.type = "Lakitu";
                } else if (dynamic_cast<model::Spiny*>(enemy)) {
                    esd.type = "Spiny";
                } else if (dynamic_cast<model::Bowser*>(enemy)) {
                    esd.type = "Bowser";
                } else if (dynamic_cast<model::PiranhaPlant*>(enemy)) {
                    esd.type = "PiranhaPlant";
                } else {
                    esd.type = "Goomba";
                }
                outLevelSave.enemies.push_back(esd);
            }
            continue;
        }

        // Item: active collectibles
        if (auto* item = dynamic_cast<model::Item*>(e.get())) {
            if (item->isActive && item->isAlive() && !item->isDying()) {
                model::ItemSaveData isd;
                isd.posX = item->getPosition().x;
                isd.posY = item->getPosition().y;
                isd.velX = item->getVelocity().x;
                isd.velY = item->getVelocity().y;
                isd.direction = item->getDirection();

                if (dynamic_cast<model::Mushroom*>(item)) {
                    isd.type = "Mushroom";
                } else if (dynamic_cast<model::FireFlower*>(item)) {
                    isd.type = "FireFlower";
                } else if (dynamic_cast<model::Starman*>(item)) {
                    isd.type = "Starman";
                } else if (dynamic_cast<model::Coin*>(item)) {
                    isd.type = "Coin";
                } else {
                    continue;
                }
                outLevelSave.items.push_back(isd);
            }
            continue;
        }
    }

    outLevelSave.hasEntitiesSnapshot = true;
}

void LevelScene::spawn(std::unique_ptr<model::Entity> entity) {
    if (!entity) return;
    entity->setWorld(this);
    if (auto* character = dynamic_cast<model::Character*>(entity.get())) {
        character->setWorld(model::WorldSet::forType(worldType));
        character->setMap(&map);
    }
    // A spawned entity is always awake: it was created by something already in play.
    entity->isDormant = false;
    pendingEntities.push_back(std::move(entity));
}

const model::Entity* LevelScene::getPlayer() const {
    return playerPtr;
}

void LevelScene::removeTile(std::size_t row, std::size_t column) {
    // A destroyed block's cell becomes air: the entity pass already forgets the inactive
    // entity, and air keeps the tile pass from grounding bodies on the block's old spot.
    map.setTile(row, column, '.');
}

void LevelScene::removeTilesOfType(char symbol) {
    // Sweeps the working grid, not the level's pristine copy, so a death restart rebuilds
    // the area with its bridge intact — the same way a smashed brick comes back.
    for (std::size_t row = 0; row < map.getRows(); ++row) {
        for (std::size_t column = 0; column < map.getColumns(); ++column) {
            if (map.getTile(row, column) == symbol) {
                map.setTile(row, column, '.');
            }
        }
    }
}

model::Entity* LevelScene::addEntity(std::unique_ptr<model::Entity> entity) {
    if (!entity) return nullptr;
    model::Entity* raw = entity.get();
    entity->setWorld(this);
    entities.push_back(std::move(entity));
    return raw;
}

void LevelScene::armDormancy() {
    // The frontier starts at the camera's right edge, so the opening screenful is awake
    // and everything beyond it waits to be scrolled into view.
    const float halfWidth = static_cast<float>(AppEngine::screenWidth()) / 2.0f;
    cameraX = playerPtr ? playerPtr->getPosition().x + playerPtr->getSize().x / 2.0f : halfWidth;
    activationFrontier = cameraX + halfWidth + ActivationMargin;

    for (const auto& e : entities) {
        // The player and the level's fixed furniture are never dormant: a pipe, the goal
        // or a slider must collide and draw (and, for a slider, move) from the first
        // frame, and dormancy exists to stop enemies acting off-screen, not to hide
        // terrain.
        if (e.get() == playerPtr || e->isSolid()) {
            e->isDormant = false;
            continue;
        }
        e->isDormant = e->getPosition().x > activationFrontier;
    }
}

void LevelScene::updateActivation() {
    // Monotonic: the frontier only ever moves right, so backtracking never re-arms an
    // enemy the player has already woken and walked past.
    const float halfWidth = static_cast<float>(AppEngine::screenWidth()) / 2.0f;
    activationFrontier = std::max(activationFrontier, cameraX + halfWidth + ActivationMargin);

    for (const auto& e : entities) {
        if (e->isDormant && e->getPosition().x <= activationFrontier) {
            e->isDormant = false;
        }
    }
}

LevelScene::Event LevelScene::update(float deltaTime) {
    if (cinematicActive) {
        return Event::None;
    }

    // Pipe travel freezes the world: the transition drives the player directly
    // (slide in, teleport, slide out) and everything else stands still.
    if (pipePhase != PipePhase::None) {
        advancePipeTransition(deltaTime);
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
        // Dormant entities do not update, collide or draw — they are placed but not yet
        // woken by the camera.
        if (!e->isActive || e->isDormant) continue;

        // Input gathering is delegated polymorphically: only the player reacts (input is
        // a Character capability — static world objects have no input). It runs BEFORE
        // entity->update() so gravity & integration see the correct player-intended
        // velocity, not stale values.
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            model::InputSnapshot snapshot;
            if (character == playerPtr) {
                if (inputMapper) {
                    snapshot.moveLeft = inputMapper->isActionPressed(model::InputAction::MoveLeft);
                    snapshot.moveRight = inputMapper->isActionPressed(model::InputAction::MoveRight);
                    snapshot.jump = inputMapper->isActionPressed(model::InputAction::Jump);
                    snapshot.run = inputMapper->isActionPressed(model::InputAction::Run);
                    snapshot.fire = inputMapper->isActionPressed(model::InputAction::Attack);
                    snapshot.crouch = inputMapper->isActionPressed(model::InputAction::Crouch);
                } else {
                    snapshot.moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
                    snapshot.moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
                    snapshot.jump = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
                    snapshot.run = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
                    snapshot.fire = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
                    snapshot.crouch = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
                }
            }
            character->handleInput(deltaTime, snapshot);
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

    // Splice in anything spawned during this frame's update/collision passes. Deferred to
    // here because growing `entities` while the loops above iterate it invalidates them.
    if (!pendingEntities.empty()) {
        for (auto& pending : pendingEntities) {
            entities.push_back(std::move(pending));
        }
        pendingEntities.clear();
    }

    // Recompute the camera and wake anything that has scrolled into range. Done at the end
    // of update() so render() and next frame's activation check agree on where the view is.
    {
        const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
        const float halfWidth = static_cast<float>(AppEngine::screenWidth()) / 2.0f;
        float target = playerPtr
            ? playerPtr->getPosition().x + playerPtr->getSize().x / 2.0f
            : halfWidth;
        cameraX = std::clamp(target, halfWidth, std::max(halfWidth, mapWidth - halfWidth));
        updateActivation();
    }

    // The player's death fall is over: the owner either ends the run or restarts.
    if (playerFinishedDeathFall) {
        return Event::RunEnded;
    }

    // Pipe entry: holding Down on a vertical pipe's cap, or merely touching a horizontal
    // one, starts the slide-in/out travel to the portal bound to that pipe's column.
    if (playerPtr) {
        if (const model::Portal* portal =
                portals.findEntryPortal(*playerPtr, level, currentArea, entities)) {
            beginPipeTransition(*portal);
        }
    }

    // Flagpole touch: report the event so the owner starts the scripted clear play.
    if (completion.isTouched() && playerPtr && !playerPtr->isDying()) {
        return Event::ClearTriggered;
    }

    // Goal touch: report the event so the owner freezes the scene and shows the overlay.
    if (goalPtr && goalPtr->isTouched() && playerPtr && !playerPtr->isDying()) {
        return Event::ClearTriggered;
    }

    return Event::None;
}

void LevelScene::render(sf::RenderTarget& window) {
    // Camera: follows the player horizontally, but never pans past the map fringes; it is
    // fixed vertically. The view keeps the fixed viewport set by AppEngine.
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float halfWidth = static_cast<float>(AppEngine::screenWidth()) / 2.0f;
    // update() already resolved the camera for this frame; recompute the clamp here only
    // so a render before the first update (or while a cinematic freezes update) is sane.
    float cameraX = this->cameraX;
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
    cameraView.setSize({static_cast<float>(AppEngine::screenWidth()),
                        static_cast<float>(AppEngine::ScreenHeight)});
    cameraView.setCenter({cameraX, static_cast<float>(AppEngine::ScreenHeight) / 2.0f});
    window.setView(cameraView);

    // The world's theme decides the background.
    window.clear(model::WorldSet::forType(worldType).getBackgroundColor());

    // World space: the tile map, then every active entity through its registered
    // renderer (no type checks here — the view dispatches polymorphically).
    if (!entityRenderers && mapLoaded && renderer) {
        renderer->render(window, map);
    }
    if (entityRenderers) {
        const view::RenderContext ctx{worldType};
        // Two passes so entities that hide behind terrain (a Piranha Plant sliding out of
        // its pipe) are covered by the tile map instead of floating in front of it.
        for (const auto& e : entities) {
            if (e->isActive && !e->isDormant && e->drawsBehindTerrain()) {
                entityRenderers->render(window, *e, ctx);
            }
        }
        if (mapLoaded && renderer) {
            renderer->render(window, map);
        }
        for (const auto& e : entities) {
            if (e->isActive && !e->isDormant && !e->drawsBehindTerrain()) {
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
    return completion.flagPole();
}

float LevelScene::castleDoorX() const {
    return completion.castleDoorX(map);
}

void LevelScene::setCinematicActive(bool active) {
    cinematicActive = active;
}

int LevelScene::getRemainingTime() const {
    return timer.getRemainingSeconds();
}

void LevelScene::pauseTimer() {
    timer.pause();
}

void LevelScene::toggleHitboxes() {
    showHitboxes = !showHitboxes;
}

}