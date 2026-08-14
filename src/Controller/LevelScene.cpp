#include "Controller/LevelScene.h"

#include "Controller/AppEngine.h"
#include "Model/Block/BrickBlock.h"
#include "Model/Block/CoinBlock.h"
#include "Model/Character.h"
#include "Model/Core/GameManager.h"
#include "Model/Enemy/Bowser.h"
#include "Model/Enemy/EnemyFactory.h"
#include "Model/Enemy/HammerBro.h"
#include "Model/Enemy/Lakitu.h"
#include "Model/Enemy/PiranhaPlant.h"
#include "Model/Enemy/Spiny.h"
#include "Model/Item/Coin.h"
#include "Model/Item/FireFlower.h"
#include "Model/Item/Mushroom.h"
#include "Model/Item/Starman.h"
#include "Model/Projectile/Fireball.h"
#include "Model/Projectile/Hammer.h"
#include "Model/Projectile/MarioFireball.h"
#include "Model/Projectile/SpinyEgg.h"
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
#include "View/Base/AtlasFrameRenderer.h"
#include "View/Enemy/FireballRenderer.h"
#include "View/Item/ItemFrameRenderer.h"
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
    entityRenderers->registerRenderer<model::FlagPole, view::FlagPoleRenderer>();
    entityRenderers->registerRenderer<model::Pipe, view::PipeRenderer>();

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
    entityRenderers->registerRenderer<model::Hammer,
                                      view::AtlasFrameRenderer<model::Hammer>>(view::atlas::Hammer);
    entityRenderers->registerRenderer<model::SpinyEgg,
                                      view::AtlasFrameRenderer<model::SpinyEgg>>(view::atlas::SpinyEgg);
    entityRenderers->registerRenderer<model::Fireball,
                                      view::AtlasFrameRenderer<model::Fireball>>(view::atlas::BowserFire);
    // Mario's fireball is its own animated ball (4 rolling frames), unlike Bowser's flat
    // breath above, so it needs its own renderer.
    entityRenderers->registerRenderer<model::MarioFireball, view::FireballRenderer>();
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
    } else {
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
        map.padRight(LevelCompletion::LevelPaddingTiles);
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
// 'M' = Mario, 'C' = CoinBlock, '#'/'B' = BrickBlock. Enemy markers are the digits 0-9
// (EnemyFactory ids), placed in the cell directly above the ground: every enemy's feet
// rest on that marker cell's bottom edge, so a body taller than one tile is dropped by
// its overhang (see the digit loop below). Digits are stripped to empty tiles at load,
// so a marker never doubles as terrain. Called on enter and after every death (the
// whole level restarts).
void LevelScene::resetLevel() {
    const std::size_t tileWidth = model::TileMap::TileWidth;
    const std::size_t tileHeight = model::TileMap::TileHeight;
    const std::size_t rows = map.getRows();
    const std::size_t columns = map.getColumns();

    entities.clear();
    playerPtr = nullptr;
    completion.clear();

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
                        auto mario = std::make_unique<model::Mario>(position);
                        playerPtr = mario.get();
                        entities.push_back(std::move(mario));
                        marioSpawned = true;
                    }
                    break;
                case 'C':
                    entities.push_back(std::make_unique<model::CoinBlock>(position, size));
                    break;
                case '#':
                case 'B':
                    entities.push_back(std::make_unique<model::BrickBlock>(position, size));
                    break;
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

    // Fallback: if the map has no 'M', keep the game playable with a fixed spawn.
    if (!marioSpawned) {
        const float groundY = static_cast<float>((rows - 2) * tileHeight - tileHeight);
        auto mario = std::make_unique<model::Mario>(
            model::Vector2{static_cast<float>(2 * tileWidth), groundY});
        playerPtr = mario.get();
        entities.push_back(std::move(mario));
    }

    // Enemies placed as digit markers (EnemyFactory ids). These are stripped to empty
    // tiles at load, so they never double as terrain; the factory is the only place an
    // enemy is constructed for a level. (The old letter markers 'E'/'K' were retired —
    // the debug maps now use the same digits as the feat maps.)
    for (const model::SpawnPoint& spawn : map.getSpawnPoints()) {
        const model::Vector2 origin = model::TileMap::tileOrigin(spawn.row, spawn.column);
        if (auto enemy = model::EnemyFactory::create(spawn.id, origin)) {
            enemy->setMap(&map);  // for ledge detection
            entities.push_back(std::move(enemy));
        }
    }

    // Level completion zone, in the padded columns: flagpole, then the goal castle.
    // (Guard inside build: with a failed load columns is 0 and there is nothing to
    // spawn.)
    completion.build(map, entities);

    // Every character obeys the current world's physics (gravity/fall/drag, swim), and
    // every entity gets this scene as its spawn channel (model::World) so emitters can
    // put projectiles and rewards into the level without knowing a controller exists.
    const model::WorldTheme& world = model::WorldSet::forType(worldType);
    for (const auto& e : entities) {
        e->setWorld(this);
        if (auto* character = dynamic_cast<model::Character*>(e.get())) {
            character->setWorld(world);
        }
    }

    // Everything ahead of the camera starts asleep; the player is always awake.
    armDormancy();
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
        // The player and the level's fixed furniture are never dormant: a pipe or the
        // flagpole must collide and draw from the first frame, and dormancy exists to
        // stop enemies acting off-screen, not to hide terrain.
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
        // Dormant entities do not update, collide or draw — they are placed but not yet
        // woken by the camera.
        if (!e->isActive || e->isDormant) continue;

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

    // Pipe entry: holding Down while standing on a pipe's cap and a portal is bound to
    // that pipe's column teleports the player to the portal's area.
    if (playerPtr) {
        if (const model::Portal* portal =
                portals.findEntryPortal(*playerPtr, level, currentArea, entities)) {
            teleportToPortal(*portal);
        }
    }

    // Flagpole touch: report the event so the owner starts the scripted clear play.
    if (completion.isTouched() && playerPtr && !playerPtr->isDying()) {
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
    return completion.castleDoorX(map);
}

}