#include "Controller/PlayState.h"

#include "Controller/AppEngine.h"
#include "Controller/GameOverState.h"
#include "Controller/MenuState.h"
#include "Controller/StateManager.h"
#include "Model/Core/GameManager.h"
#include "Model/Player/Mario.h"
#include "Model/Player/Luigi.h"
#include "Model/Player/Player.h"
#include "Model/Enemy/Bowser.h"
#include "Model/Enemy/EnemyFactory.h"
#include "Model/Enemy/Goomba.h"
#include "Model/Enemy/HammerBro.h"
#include "Model/Enemy/Koopa.h"
#include "Model/Enemy/Lakitu.h"
#include "Model/Enemy/Spiny.h"
#include "Model/Projectile/Fireball.h"
#include "Model/Projectile/Hammer.h"
#include "Model/Projectile/MarioFireball.h"
#include "Model/Projectile/SpinyEgg.h"
#include "Model/Block/CoinBlock.h"
#include "Model/Item/FireFlower.h"
#include "Model/Item/Coin.h"
#include "Model/Item/Mushroom.h"
#include "Model/Item/Starman.h"
#include "View/Base/AtlasFrameRenderer.h"
#include "View/Enemy/EnemyAtlas.h"
#include "View/Enemy/FireballRenderer.h"
#include "View/Item/ItemAtlas.h"
#include "View/Item/ItemFrameRenderer.h"
#include "Model/Enemy/PiranhaPlant.h"
#include "Model/Enemy/Spiny.h"
#include "Model/Projectile/Fireball.h"
#include "Model/Projectile/Hammer.h"
#include "Model/Projectile/SpinyEgg.h"
#include "Model/Block/CoinBlock.h"
#include "View/Base/AtlasFrameRenderer.h"
#include "View/Enemy/EnemyAtlas.h"
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
    // Levels that have been built get their own file; anything else falls back to the
    // scratch map used for trying features out.
    switch (level) {
        case 1:  return "assets/maps/level1_1.map";
        default: return "assets/maps/debug.map";
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

    // Build the view: one renderer per entity type + the screen-space HUD.
    entityRenderers = std::make_unique<view::EntityRendererRegistry>();
    entityRenderers->registerRenderer<model::Mario, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Luigi, view::PlayerRenderer>();
    entityRenderers->registerRenderer<model::Goomba, view::GoombaRenderer>();
    entityRenderers->registerRenderer<model::Koopa, view::KoopaRenderer>();
    entityRenderers->registerRenderer<model::CoinBlock, view::CoinBlockRenderer>();

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
    pendingEntities.clear();
    player = nullptr;

    // Spawn Player — place on ground row (row 1 = y = (Rows-2)*TileHeight)
    // Ground is at tilemap rows 0-1 (bottom). In world coords bottom row 0 is at
    // y = (Rows-1)*TileHeight. Spawn Mario one tile above ground.
    const float groundY = static_cast<float>((model::TileMap::Rows - 2) * TileHeight
                                             - TileHeight);
    std::unique_ptr<model::Player> hero;
    if (playAsLuigi) {
        hero = std::make_unique<model::Luigi>(model::Vector2{64.0f, groundY});
    } else {
        hero = std::make_unique<model::Mario>(model::Vector2{64.0f, groundY});
    }
    player = hero.get();
    addEntity(std::move(hero));

    // Hostiles come entirely from the map. Nothing here decides where an enemy goes: the
    // level author writes a digit into the map file and the factory turns it into an object.
    for (const model::SpawnPoint& point : map.getSpawnPoints()) {
        const model::Vector2 origin = model::TileMap::tileOrigin(point.row, point.column);
        if (auto enemy = model::EnemyFactory::create(point.id, origin)) {
            addEntity(std::move(enemy));
        }
    }

    // Coin blocks come from the map too: every 'C' the level author wrote becomes a working
    // block (bump -> coin / mushroom / flower). The tile is stripped during load, so the
    // entity is the only thing occupying that cell.
    for (const model::SpawnPoint& point : map.getCoinBlockSpawns()) {
        const model::Vector2 origin = model::TileMap::tileOrigin(point.row, point.column);
        addEntity(std::make_unique<model::CoinBlock>(
            origin,
            model::Vector2{static_cast<float>(model::TileMap::TileWidth),
                           static_cast<float>(model::TileMap::TileHeight)}));
    }

    // Seed the camera from the freshly placed player, then put everything the view has not
    // reached yet to sleep. Order matters: the frontier has to exist before it can be
    // applied, and it restarts with the level so a retry re-arms every enemy.
    activationFrontier = 0.0f;
    updateCamera();
    armDormancy();

    // The clock is a per-attempt allowance: a retry after a death starts from full, it does
    // not inherit whatever was left when the last life ran out.
    model::GameManager::instance().startLevelTimer();
}

// Camera: follows the player horizontally, but never pans past the map fringes; it is fixed
// vertically. Runs at the END of update() so the position it reports is the settled one for
// this frame — render() draws with it, and the next frame's wake check reads the same value.
void PlayState::updateCamera() {
    const float mapWidth = static_cast<float>(map.getColumns()) * model::TileMap::TileWidth;
    const float halfWidth = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;

    float centre = halfWidth;
    if (player) {
        centre = player->getPosition().x + player->getSize().x / 2.0f;
    }
    centre = std::clamp(centre, halfWidth, std::max(halfWidth, mapWidth - halfWidth));
    // Snap the camera to a whole logical pixel. The world is composited into an offscreen
    // target at the logical resolution and upscaled once, so logical pixels *are* the grid
    // that matters here: integer camera positions keep every tile edge aligned (no seams)
    // while the scroll rate stays perfectly even (WindowScale never enters this maths).
    cameraX = std::round(centre);

    // The frontier only ever advances. Backtracking must not send woken enemies back to
    // sleep, and an entity that outruns the camera (a kicked shell) must stay awake.
    activationFrontier = std::max(activationFrontier, cameraX + halfWidth + ActivationMargin);
}

// Put every entity the camera has not reached yet to sleep. Called once per level build; the
// map-driven spawner will inherit this for free rather than having to flag enemies itself.
void PlayState::armDormancy() {
    for (auto& e : entities) {
        if (e.get() == player) continue;  // the player is never dormant
        e->isDormant = e->getPosition().x > activationFrontier;
    }
}

model::Entity* PlayState::addEntity(std::unique_ptr<model::Entity> entity) {
    if (!entity) return nullptr;
    entity->setWorld(this);
    model::Entity* raw = entity.get();
    entities.push_back(std::move(entity));
    return raw;
}

// model::World. Called from inside the update loop (a Hammer Bro throwing, a Spiny Egg
// hatching), so the entity is queued rather than appended: growing `entities` mid-iteration
// would invalidate the loop walking it.
void PlayState::spawn(std::unique_ptr<model::Entity> entity) {
    if (!entity) return;
    entity->setWorld(this);
    // Spawned entities are born awake — they are created at the action, not placed ahead of
    // the camera the way map enemies are.
    entity->isDormant = false;
    pendingEntities.push_back(std::move(entity));
}

const model::Entity* PlayState::getPlayer() const {
    return player;
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
            case sf::Keyboard::Key::C:
                // Character switch: restart the level as the other brother. The fresh run
                // keeps the same lives/score, only the character changes.
                playAsLuigi = !playAsLuigi;
                resetLevel();
                break;
            case sf::Keyboard::Key::H:
                // Debug: toggle the collision-box overlay.
                showHitboxes = !showHitboxes;
            case sf::Keyboard::Key::N:
                // Debug stand-in for finishing a level: banks the unspent clock as score and
                // restarts. Replace this with the real goal trigger once one exists.
                model::GameManager::instance().awardTimeBonus();
                resetLevel();
                break;
            default:
                break;
        }
    }
}

void PlayState::update(float deltaTime) {
    // The clock only runs while the level is actually being played: it freezes during the
    // death fall so the timer cannot expire on a body that is already falling.
    model::GameManager& game = model::GameManager::instance();
    if (player && !player->isDying()) {
        game.tickTimer(deltaTime);
        if (game.isTimeUp()) {
            // Out of time is fatal, as in the original. It costs a life and restarts the
            // level through the same death flow as any other way of dying.
            player->die(false);
        }
    }

    // Wake pass: anything the frontier has swept past joins the simulation from now on.
    // Uses the frontier settled at the end of the previous frame.
    for (auto& e : entities) {
        if (e->isDormant && e->getPosition().x <= activationFrontier) {
            e->isDormant = false;
        }
    }

    std::vector<model::Entity*> activeEntities;
    for (auto& e : entities) {
        if (!e->isActive || e->isDormant) continue;

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
        // Dormant entities are exempt: they have not moved, so they cannot have left the
        // world, and the despawn rules must not reclaim them before they ever wake.
        if (!e->isActive || e->isDormant) continue;
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
        } else if (e->hitbox.layer == model::CollisionLayer::Projectile) {
            // Projectiles are bound to the camera, not the map: a hammer thrown at the
            // screen edge should not linger off-screen for the rest of the level.
            // Horizontal only, deliberately — a hammer arcs *above* the top of the view at
            // the peak of its throw, and a full screen-rect test would delete it mid-flight.
            const float halfWidth = static_cast<float>(AppEngine::ScreenWidth) / 2.0f;
            const float viewLeft = cameraX - halfWidth - DespawnMargin;
            const float viewRight = cameraX + halfWidth + DespawnMargin;
            if (pos.x + sz.x < viewLeft || pos.x > viewRight || pos.y > mapHeight) {
                e->isActive = false;
            }
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
            resetLevel();  // rebuilds the camera and re-arms dormancy itself
        }
        return;
    }

    // Splice in anything spawned during this frame. Done here, after every loop over
    // `entities` has finished, because appending mid-iteration invalidates them.
    for (auto& spawned : pendingEntities) {
        entities.push_back(std::move(spawned));
    }
    pendingEntities.clear();

    // Settle the camera last, once every position for this frame is final.
    updateCamera();
}

void PlayState::render(sf::RenderTarget& window) {
    // Camera position is computed in update() (see updateCamera) and simply read back here,
    // so the view keeps the fixed viewport set by AppEngine.
    const sf::View baseView = window.getView();
    sf::View cameraView = baseView;
    cameraView.setSize({static_cast<float>(AppEngine::ScreenWidth),
                        static_cast<float>(AppEngine::ScreenHeight)});
    cameraView.setCenter({cameraX, static_cast<float>(AppEngine::ScreenHeight) / 2.0f});
    window.setView(cameraView);

    const sf::Color skyBlue(92, 148, 252);
    window.clear(skyBlue);

    // World space, back to front: entities that live behind the terrain, then the tile map
    // over them, then everything else. No type checks here — the view dispatches
    // polymorphically and the entity itself declares which side of the terrain it is on.
    const auto drawEntities = [&](bool behindTerrain) {
        if (!entityRenderers) return;
        for (const auto& e : entities) {
            if (!e->isActive || e->isDormant) continue;
            if (e->drawsBehindTerrain() != behindTerrain) continue;
            entityRenderers->render(window, *e);
        }
    };

    drawEntities(true);
    if (mapLoaded && renderer) {
        renderer->render(window, map);
    }
    drawEntities(false);

    // Debug overlay: hitboxes go on top of the sprites, still in world space so they line
    // up with what they bound. Solid tiles first, then entities over them.
    if (showHitboxes) {
        if (mapLoaded) {
            hitboxRenderer.renderTiles(window, map);
        }
        for (const auto& e : entities) {
            if (e->isActive && !e->isDormant) {
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
