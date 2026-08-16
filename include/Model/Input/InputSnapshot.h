#ifndef MODEL_INPUT_SNAPSHOT_H
#define MODEL_INPUT_SNAPSHOT_H

namespace model {

// POD struct representing the state of inputs required for character/player movement.
// Built each frame by LevelScene (Controller layer) from IInputMapper or raw keyboard
// fallback, then passed down into Character::handleInput — keeping the Model layer
// free of any direct input-system dependency.
struct InputSnapshot {
    bool moveLeft  = false;
    bool moveRight = false;
    bool jump      = false;
    bool run       = false;  // sprint modifier; also used as the fire trigger in some mappings
    bool fire      = false;  // shoot fireball (Fire power only)
    bool crouch    = false;  // hold-down intent: enters pipes, crouches
};

} // namespace model

#endif // MODEL_INPUT_SNAPSHOT_H
