#ifndef MODEL_VECTOR2_H
#define MODEL_VECTOR2_H

namespace model {

struct Vector2 {
    // Default member initializers: a default-constructed Vector2 is always (0,0),
    // never indeterminate. Aggregate initialization (Vector2{a, b}) is unaffected.
    float x = 0.0f;
    float y = 0.0f;
};

}

#endif
