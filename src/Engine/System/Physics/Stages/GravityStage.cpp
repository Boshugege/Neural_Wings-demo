#ifdef _MSC_VER
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#pragma warning(disable : 4305)
#endif

#include "GravityStage.h"
#include "Engine/Core/GameWorld.h"
#include "Engine/Core/Components/RigidBodyComponent.h"
#include "Engine/Core/Components/TransformComponent.h"

#include "Engine/Utils/JsonParser.h"
#include <iostream>

GravityStage::GravityStage(Vector3f gravity) : m_gravity(gravity) {}
void GravityStage::Initialize(const json &config)
{
    if (config.contains("gravity"))
    {
        m_gravity = JsonParser::ToVector3f(config["gravity"]);
    }
    ground = config.value("ground", 0.0);
    e_ground = config.value("e_ground", 0.5);
    mu = config.value("mu", 0.1);
}
void GravityStage::Execute(GameWorld &world, float fixedDeltaTime)
{
    auto &gameObjects = world.GetActivateGameObjects();
    if (gameObjects.empty())
    {
        std::cout << "[Gravity Stage]:Empty Game World" << std::endl;
        return;
    }
    for (auto &gameObject : gameObjects)
    {
        if (gameObject->HasComponent<RigidbodyComponent>())
        {
            auto &rb = gameObject->GetComponent<RigidbodyComponent>();
            if (rb.mass <= 0.001f)
                continue;
            auto &tf = gameObject->GetComponent<TransformComponent>();
            rb.AddForce(m_gravity * rb.mass);
            Vector3f corners[8];

            float lowy = 0.0f;
            if (rb.colliderType == ColliderType::BOX)
            {
                AABB aabb = gameObject->GetWorldAABB(&corners);
                lowy = aabb.min.y();
            }
            else if (rb.colliderType == ColliderType::SPHERE)
            {
                AABB aabb = gameObject->GetWorldAABB();
                lowy = aabb.min.y();
            }
            Vector3f normal = Vector3f(0.0f, 1.0f, 0.0f);
            if (lowy < ground)
            {
                float penetration = ground - lowy;
                if (penetration > slop)
                {
                    Vector3f pos = tf.GetWorldPosition();
                    pos.y() += (penetration - slop) * baumgarte;
                    tf.SetWorldPosition(pos);
                }
                struct Contact
                {
                    Vector3f r;
                    float penetation;
                };
                std::vector<Contact> contacts;
                if (rb.colliderType == ColliderType::BOX)
                    for (size_t i = 0; i < 8; i++)
                    {
                        if (corners[i].y() < ground + 0.01f)
                        {
                            contacts.push_back({corners[i] - tf.GetWorldPosition(), ground - corners[i].y()});
                        }
                    }
                else if (rb.colliderType == ColliderType::SPHERE)
                {
                    contacts.push_back({Vector3f(0.0f, -rb.boudingRadius, 0.0f), ground - tf.GetWorldPosition().y() - rb.boudingRadius});
                }

                const float div = (float)contacts.size();
                for (auto &cp : contacts)
                {
                    Vector3f rV = rb.velocity + (rb.angularVelocity ^ cp.r);
                    float nrV = rV * normal;
                    if (nrV < -0.01f)
                    {
                        float invMass = 1.0f / rb.mass;
                        float e = (fabsf(nrV) < 0.2f) ? 0.0f : rb.elasticity * e_ground;
                        float i = -(1.0f + e) * nrV;
                        auto raxn = cp.r ^ normal;
                        auto rot = tf.GetWorldRotation().toMatrix();
                        auto worldInverseInertia = rot * rb.inverseInertiaTensor * rot.transposed();
                        float term = raxn * (worldInverseInertia * raxn);
                        float j = i / (term + invMass);
                        auto impulse = j * normal / div;
                        rb.AddImpulse(impulse, cp.r);

                        // 摩擦力冲量
                        Vector3f tangent = rV - (normal * nrV);
                        if (tangent.LengthSquared() > 0.01f)
                        {
                            tangent.Normalize();
                            float vt = rV * tangent;
                            Vector3f raxt = cp.r ^ tangent;
                            float angularTermT = raxt * (worldInverseInertia * raxt);
                            float jt = -vt / (invMass + angularTermT);
                            jt = std::max(-j * mu, std::min(j * mu, jt));

                            Vector3f impulseT = jt * tangent / div;
                            rb.AddImpulse(impulseT, cp.r);
                        }
                    }
                }
                // 防止不稳定
                if (rb.velocity.LengthSquared() < 0.1f && rb.angularVelocity.LengthSquared() < 0.1f)
                {
                    rb.velocity.y() = 0.0f;

                    if (contacts.size() >= 3)
                    {
                        rb.velocity = Vector3f::ZERO;
                        rb.angularMomentum = Vector3f::ZERO;
                    }
                }
            }
        }
    }
}
