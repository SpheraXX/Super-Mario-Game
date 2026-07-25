#ifndef MODEL_BLOCK_H
#define MODEL_BLOCK_H

#include "Model/Entity.h"

namespace model {

class Block : public Entity {
public:
    Block(Vector2 position, Vector2 size, char tileSymbol);

    char getTileSymbol() const;
    bool isSolid() const;

private:
    char tileSymbol;
    bool solid;
};

}

#endif
