#include "Model/Block.h"

namespace model {

Block::Block(Vector2 position, Vector2 size, char tileSymbol)
    : Entity(position, size), tileSymbol(tileSymbol), solid(tileSymbol != '.') {
}

char Block::getTileSymbol() const {
    return tileSymbol;
}

bool Block::isSolid() const {
    return solid;
}

}
