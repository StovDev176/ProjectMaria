#pragma once
#include "Math.hpp"
#include "ECS.hpp"
#include "Components.hpp"
#include <memory>
#include <iostream>
#include <optional>
#include <vector>
#include <cmath>
#include "raylib.h"

struct Raycast {
  vector3 origin;
  vector3 direction;
  Raycast(const vector3& origin, const vector3& direction) : origin(origin), direction(direction) {}
};

struct Shape {
  virtual ~Shape() = default;
  virtual std::optional<vector3> intersect(const Raycast& ray) const = 0;
};

struct Sphere : Shape {  
  vector3 center;
  float radius;

  Sphere(const vector3& center, float radius) : center(center), radius(radius) {}

  std::optional<vector3> intersect(const Raycast& ray) const override {
    vector3 dir = ray.direction.Unit();
    vector3 L = center - ray.origin;
    float tca = L.Dot(dir);

    if (tca < 0) return std::nullopt;
    
    float d2 = L.Magnitude() * L.Magnitude() - tca * tca;
    if (d2 > radius * radius) return std::nullopt;
    
    float thc = std::sqrt(radius * radius - d2);
    float t0 = tca - thc;
    
    return ray.origin + (dir * t0);
  }
};

struct Quadrilateral : Shape {
  vector3 size;
  vector3 pos;

  Quadrilateral(const vector3& size, const vector3& pos) : pos(pos), size(size) {}
};

class Scene {
private:    
    std::vector<std::unique_ptr<Shape>> shapes;
public:
    void addShape(std::unique_ptr<Shape> shape) {
        shapes.push_back(std::move(shape));
    }
    
    std::optional<vector3> castRay(const Raycast& ray) const {
        for (const auto& s : shapes) {
            if (auto point = s->intersect(ray)) {
                return point;
            }
        }
        return std::nullopt;
    }
};

struct RigidBody {
    vector3 velocity;
    vector3 acceleration;
    float mass;
    float bounciness;
    vector3 netForce;
};

struct TransformComponent {
    vector3 position;
    vector3 rotation;
    vector3 scale;
    Matrix4 worldMatrix = Matrix4::Identity();
    Matrix4 localMatrix = Matrix4::Identity();
};

struct AABB {
    vector3 min; 
    vector3 max; 
};

struct Manifold {
    vector3 normal;     
    float penetration;
    bool colliding;
};

Manifold computeManifold(const AABB& a, const AABB& b) {
    Manifold m;
    m.colliding = false;

    float overlapX = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
    float overlapY = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);
    float overlapZ = std::min(a.max.z, b.max.z) - std::max(a.min.z, b.min.z);

    if (overlapX <= 0.0f || overlapY <= 0.0f || overlapZ <= 0.0f) {
        return m;
    }

    m.colliding = true;

    if (overlapX < overlapY && overlapX < overlapZ) {
        m.penetration = overlapX;
        m.normal = vector3((a.max.x < b.max.x) ? -1.0f : 1.0f, 0.0f, 0.0f);
    } else if (overlapY < overlapZ) {
        m.penetration = overlapY;
        m.normal = vector3(0.0f, (a.max.y < b.max.y) ? -1.0f : 1.0f, 0.0f);
    } else {
        m.penetration = overlapZ;
        m.normal = vector3(0.0f, 0.0f, (a.max.z < b.max.z) ? -1.0f : 1.0f);
    }

    return m;
}

inline float invMass(float mass) {
    return (mass > 0.0f) ? (1.0f / mass) : 0.0f;
}


bool checkCollision(AABB a, AABB b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

void applyGravity(RigidBody& rb) {
  vector3 gravityForce = vector3(0, (-9.81f * rb.mass), 0);
  rb.netForce += gravityForce;
}

void applyForce(RigidBody& rb, vector3 force) {
  rb.acceleration += (force/rb.mass);
}

void updatePhysics(RigidBody& rb,TransformComponent& tfC,  float dt) {
  rb.acceleration = vector3(0, 0, 0);

  if (rb.mass > 0.0f) {
    applyForce(rb, rb.netForce);
  }
  rb.velocity += (rb.acceleration * dt);
  tfC.position += (rb.velocity * dt);

  rb.netForce = vector3(0, 0, 0);
}

void collide(RigidBody& rb1, RigidBody& rb2, const vector3& normal, float restitution) {
    vector3 relativeVelocity = rb1.velocity - rb2.velocity;
    float velAlongNormal = relativeVelocity.Dot(normal);
    if (velAlongNormal > 0.0f) return;

    float invMassA = invMass(rb1.mass);
    float invMassB = invMass(rb2.mass);

    if (invMassA + invMassB == 0.0f) return;

    const float restitutionThreshold = 0.1f;
    float e = (std::abs(velAlongNormal) < restitutionThreshold) ? 0.0f : restitution;

    float impulseScalar = -(1.0f + e) * velAlongNormal / (invMassA + invMassB);
    vector3 impulse = normal * impulseScalar;

    rb1.velocity += impulse * invMassA;
    rb2.velocity -= impulse * invMassB;
}

void applyFriction(RigidBody& rb, float frictionCoefficient, float dt) {
    float factor = std::pow(frictionCoefficient, dt);
    rb.velocity.x *= factor;
    rb.velocity.z *= factor;
}

void posCorrection(const Manifold& m, RigidBody& rb1, RigidBody& rb2,
                    TransformComponent& t1, TransformComponent& t2) {
    const float slop = 0.01f;   
    const float percent = 0.8f; 

    float invMassA = invMass(rb1.mass);
    float invMassB = invMass(rb2.mass);
    if (invMassA + invMassB == 0.0f) return;

    float correctionMag = std::max(m.penetration - slop, 0.0f) / (invMassA + invMassB) * percent;
    vector3 correction = m.normal * correctionMag;

    t1.position += correction * invMassA;
    t2.position -= correction * invMassB;
}

AABB createAABB(const TransformComponent& tf) {
    AABB box;
    vector3 halfScale = tf.scale * 0.5f; 
    box.min = tf.position - halfScale;
    box.max = tf.position + halfScale;
    return box;
}

void Update(Registry& registry) {
    struct BodyData { Entity id; TransformComponent& tfc; RigidBody& rb; };
    std::vector<BodyData> bodies;
    registry.view<TransformComponent, RigidBody>([&](Entity id, TransformComponent& tfc, RigidBody& rb) {
        bodies.push_back({id, tfc, rb});
    });
    for (size_t i = 0; i < bodies.size(); ++i) {
        AABB boxA = createAABB(bodies[i].tfc);

        for (size_t j = i + 1; j < bodies.size(); ++j) {
            AABB boxB = createAABB(bodies[j].tfc);
            Manifold m = computeManifold(boxA, boxB);

            if (m.colliding) {
                float res = std::min(bodies[i].rb.bounciness, bodies[j].rb.bounciness);
                collide(bodies[i].rb, bodies[j].rb, m.normal, res);
                posCorrection(m, bodies[i].rb, bodies[j].rb, bodies[i].tfc, bodies[j].tfc);
            }
        }
    }
}