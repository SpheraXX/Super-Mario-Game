#ifndef MODEL_NPC_H
#define MODEL_NPC_H

#include "Model/Character.h"

#include <string>

namespace model {

class Player;

class NPC : public Character {
public:
    NPC(Vector2 position, Vector2 size, const std::string& dialogue);
    ~NPC() override = default;

    virtual void interact(Player& player);

    std::string getDialogue() const;
    bool isInteractable() const;
    void setInteractable(bool value);

protected:
    std::string dialogue;
    bool interactable;
};

}

#endif
