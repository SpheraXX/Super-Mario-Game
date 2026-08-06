#ifndef MODEL_MUSHROOMRETAINER_H
#define MODEL_MUSHROOMRETAINER_H

#include "Model/NPC/NPC.h"

namespace model {

class MushroomRetainer : public NPC {
public:
    MushroomRetainer(Vector2 position);

    void interact(Player& player) override;

private:
    static constexpr float DefaultWidth = 16.0f;
    static constexpr float DefaultHeight = 16.0f;
};

}

#endif
