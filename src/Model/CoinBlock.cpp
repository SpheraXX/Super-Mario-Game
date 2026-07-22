#include "Model/CoinBlock.h"

namespace model {

CoinBlock::CoinBlock(Vector2 position, Vector2 size)
    : Block(position, size, 'C'), coinAvailable(true) {
}

bool CoinBlock::hasCoin() const {
    return coinAvailable;
}

void CoinBlock::collectCoin() {
    coinAvailable = false;
}

}
