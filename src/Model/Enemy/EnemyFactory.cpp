#include "Model/Enemy/EnemyFactory.h"

#include "Model/Enemy/Bowser.h"
#include "Model/Enemy/Enemy.h"
#include "Model/Enemy/Goomba.h"
#include "Model/Enemy/HammerBro.h"
#include "Model/Enemy/Koopa.h"
#include "Model/Enemy/Lakitu.h"
#include "Model/Enemy/PiranhaPlant.h"
#include "Model/Enemy/Spiny.h"
#include "Model/Map/TileMap.h"

#include "Model/Core/LogManager.h"

#include <iostream>

namespace model {

namespace {
// Drop an enemy so its feet sit on the bottom edge of the marker tile. Anything one tile tall
// is unaffected; Bowser (two tiles) would otherwise float.
Vector2 footAligned(Vector2 tileOrigin, const Enemy& enemy) {
    const float overhang = enemy.getSize().y - static_cast<float>(TileMap::TileHeight);
    return {tileOrigin.x, tileOrigin.y - overhang};
}

template <typename T, typename... Args>
std::unique_ptr<Enemy> make(Vector2 tileOrigin, Args&&... args) {
    auto enemy = std::make_unique<T>(tileOrigin, std::forward<Args>(args)...);
    enemy->setPosition(footAligned(tileOrigin, *enemy));
    return enemy;
}
}

std::unique_ptr<Enemy> EnemyFactory::create(int id, Vector2 tileOrigin) {
    switch (id) {
        case Goomba:          return make<model::Goomba>(tileOrigin);
        case Koopa:           return make<model::Koopa>(tileOrigin, false);
        case KoopaParatroopa: return make<model::Koopa>(tileOrigin, true);
        case HammerBro:       return make<model::HammerBro>(tileOrigin);
        case Lakitu:          return make<model::Lakitu>(tileOrigin);
        case Spiny:           return make<model::Spiny>(tileOrigin);
        case Bowser:          return make<model::Bowser>(tileOrigin);

        case PiranhaPlant:
            // The odd one out: it is not stood on the marker tile, it hangs off the pipe
            // below it. The marker goes in the empty cell directly above the pipe's top-left
            // cell — it cannot go *on* the mouth, because the loader strips a spawn digit to
            // an empty tile and that would punch a hole in the pipe.
            return std::make_unique<model::PiranhaPlant>(
                Vector2{tileOrigin.x, tileOrigin.y + TileMap::TileHeight});

        case CheepCheep:
            model::LogManager::instance().warning("EnemyFactory: enemy id " + std::to_string(id) + " is not implemented yet; skipping this spawn point.");
            return nullptr;

        default:
            model::LogManager::instance().warning("EnemyFactory: unknown enemy id " + std::to_string(id) + " in map; skipping.");
            return nullptr;
    }
}

}
