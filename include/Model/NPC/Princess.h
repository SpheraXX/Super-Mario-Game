#ifndef MODEL_PRINCESS_H
#define MODEL_PRINCESS_H

#include "Model/NPC/NPC.h"

namespace model {

class Princess : public NPC {
public:
    Princess(Vector2 position);

    void interact(Player& player) override;

private:
    static constexpr float DefaultWidth = 16.0f;
    static constexpr float DefaultHeight = 16.0f;
};

}

#endif
