#include "Model/MushroomRetainer.h"
#include "Model/Player.h"

namespace model {

MushroomRetainer::MushroomRetainer(Vector2 position)
    : NPC(position, {DefaultWidth, DefaultHeight}, "The Princess is in another castle!") {
}

void MushroomRetainer::interact(Player& player) {
    player.addScore(1000);
    interactable = false;
}

}
