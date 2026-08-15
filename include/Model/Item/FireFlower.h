#ifndef MODEL_FIREFLOWER_H
#define MODEL_FIREFLOWER_H

#include "Model/Item/Item.h"

namespace model {

// Fire Flower: sits on top of the ? block it emerged from (gravity disabled so it never
// falls). Picking it up turns a non-Fire player into Fire; an already-Fire player earns
// 1000 points instead.
class FireFlower : public Item {
public:
    FireFlower(Vector2 position);

    void onCollect(Entity& collector) override;
};

}

#endif
