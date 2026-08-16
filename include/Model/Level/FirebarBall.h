#ifndef MODEL_LEVEL_FIREBARBALL_H
#define MODEL_LEVEL_FIREBARBALL_H

#include "Model/Projectile/Projectile.h"

namespace model {

// One 8x8 flame of a firebar. A firebar is not a single entity: the map's FirebarSymbol
// spawns a LINE of these, all sharing one pivot and one angular speed, at radii 8, 16, 24…
// so the whole bar sweeps as a rigid arm. Modelling it per ball rather than per bar is
// what lets it collide at all — every collision path in the engine is an AABB test, and a
// rotating bar has no useful bounding box, while each individual flame does.
//
// A Projectile (not an Enemy) because that is exactly what it is to the player: a moving
// hazard on the Projectile layer that deals contact damage and cannot be stomped, jumped
// on, or killed. It differs from a thrown projectile in two ways, both overridden below:
// it ignores terrain (it sweeps straight through the block it is anchored to), and it does
// NOT expire on contact — a firebar keeps turning after it burns you.
class FirebarBall : public Projectile {
public:
    // `pivot` is the CENTRE of the block the bar turns on, `radius` this ball's distance
    // from it, `angularSpeed` radians/second (shared by every ball of one bar, which is
    // what keeps the arm rigid), `phase` the bar's starting angle in radians.
    FirebarBall(Vector2 pivot, float radius, float angularSpeed, float phase);

    void update(float deltaTime) override;

    // Sweeps through whatever it is mounted on; the tile pass would shove it out of its own
    // pivot block on frame one.
    bool usesTileCollision() const override { return false; }

    // Burn, but keep turning. Projectile::onCollision expires the projectile after it
    // lands a hit, which is right for a hammer and wrong for a firebar.
    void onCollision(Entity& other, CollisionType side) override;

    // Current sweep angle, for the renderer to pick a rotation frame from.
    float getSpinAngle() const { return angle; }

private:
    Vector2 pivot;
    float radius;
    float angularSpeed;
    float angle;
};

}

#endif
