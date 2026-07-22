#ifndef MODEL_ENTITY_H
#define MODEL_ENTITY_H

namespace model {

struct Vector2 {
    float x;
    float y;
};

class Entity {
public:
    Entity(Vector2 position, Vector2 size);
    virtual ~Entity() = default;

    virtual void update(float deltaTime);

    Vector2 getPosition() const;
    Vector2 getSize() const;
    void setPosition(Vector2 newPosition);

private:
    Vector2 position;
    Vector2 size;
};

}

#endif
