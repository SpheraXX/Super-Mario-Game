#ifndef MODEL_MUSHROOMRETAINER_H
#define MODEL_MUSHROOMRETAINER_H

#include "Model/NPC/NPC.h"

namespace model {

class MushroomRetainer : public NPC {
public:
    MushroomRetainer(Vector2 position);

    void interact(Player& player) override;

private:
    // Matches the artwork exactly (view::atlas::MushroomRetainer is 16x24). The frame is
    // stretched onto this box, so a box of a different shape shows up as a squashed Toad
    // rather than as anything to do with collision.
    static constexpr float DefaultWidth = 16.0f;
    static constexpr float DefaultHeight = 24.0f;
};

}

#endif
