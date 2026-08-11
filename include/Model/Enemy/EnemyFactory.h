#ifndef MODEL_ENEMYFACTORY_H
#define MODEL_ENEMYFACTORY_H

#include "Model/Core/Vector2.h"

#include <memory>

namespace model {

class Enemy;

// Turns a map digit into an enemy. This is the *only* place an enemy is constructed for a
// level, which is what keeps enemy placement entirely in the map data — no game code decides
// where a Goomba goes.
//
// A simple factory, not the GoF Factory Method: there is one creator (the map loader), so
// there is no creator hierarchy to override. The Factory Method in this codebase is
// Enemy::createProjectile, where the creator hierarchy genuinely exists.
class EnemyFactory {
public:
    enum Id {
        Goomba = 0,
        Koopa = 1,
        KoopaParatroopa = 2,
        HammerBro = 3,
        Lakitu = 4,
        Spiny = 5,
        CheepCheep = 6,   // deferred: needs the water/swimming mechanic
        Bowser = 7,
        PiranhaPlant = 8  // deferred: needs the pipe-attachment design
    };

    // `tileOrigin` is the top-left corner of the tile the marker was found in. Enemies taller
    // than one tile are dropped so their feet rest on that tile's bottom edge, which is what
    // a level author means when they put a marker on the ground line.
    //
    // Returns nullptr for ids that are not implemented yet; the caller skips those.
    static std::unique_ptr<Enemy> create(int id, Vector2 tileOrigin);
};

}

#endif
