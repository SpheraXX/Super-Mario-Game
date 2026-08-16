#include "Model/NPC/Princess.h"
#include "Model/Player/Player.h"

namespace model {

Princess::Princess(Vector2 position)
    : NPC(position, {DefaultWidth, DefaultHeight}, "Thank you Mario!") {
}

void Princess::interact(Player& player) {
    player.addScore(5000);
    interactable = false;
}

}
