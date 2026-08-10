#ifndef MODEL_STARMAN_H
#define MODEL_STARMAN_H

#include "Model/Item/Item.h"

namespace model {

// Starman (Super Star): like the Fire Flower it sits on top of the ? block it emerged
// from (gravity disabled so it never falls). Picking it up puts the player into the
// Star state — brief invulnerability during which touching any enemy defeats it;
// grabbing one while already star earns 1000 points instead.
class Starman : public Item {
public:
    Starman(Vector2 position);

    void onCollect(Entity& collector) override;
};

}

#endif
