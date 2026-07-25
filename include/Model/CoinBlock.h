#ifndef MODEL_COINBLOCK_H
#define MODEL_COINBLOCK_H

#include "Model/Block.h"

namespace model {

class CoinBlock : public Block {
public:
    CoinBlock(Vector2 position, Vector2 size);

    bool hasCoin() const;
    void collectCoin();

private:
    bool coinAvailable;
};

}

#endif
