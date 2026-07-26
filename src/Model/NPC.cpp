#include "Model/NPC.h"
#include "Model/Player.h"

namespace model {

NPC::NPC(Vector2 position, Vector2 size, const std::string& dialogueText)
    : Character(position, size),
      dialogue(dialogueText),
      interactable(true) {
}

void NPC::interact(Player& /* player */) {
}

std::string NPC::getDialogue() const {
    return dialogue;
}

bool NPC::isInteractable() const {
    return interactable;
}

void NPC::setInteractable(bool value) {
    interactable = value;
}

}
