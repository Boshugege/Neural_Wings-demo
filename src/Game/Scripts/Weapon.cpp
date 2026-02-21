#include "Weapon.h"
#include "Engine/Core/GameObject/GameObject.h"
#include "Engine/Core/GameWorld.h"
void BulletScript::OnFixedUpdate(float fixedDeltaTime)
{
    timer += fixedDeltaTime;
    if (timer >= lifeTime)
        owner->GetOwnerWorld()->GetPool("bullet").Recycle(owner);
}
void BulletScript::OnWake()
{

    timer = 0.0f;
}

void WeaponScript::Initialize(const json &data)
{
    if (data.contains("velocity"))
        m_bulletVelocity = data["velocity"];
}
void WeaponScript::OnUpdate(float deltaTime)
{
    auto &input = owner->GetOwnerWorld()->GetInputManager();
    if (input.IsActionDown("Fire"))
    {
        m_fireTimer += deltaTime;
        while (m_fireTimer >= 0.15f)
        {
            m_fireTimer -= 0.15f;
            auto &tf = owner->GetComponent<TransformComponent>();
            float size = owner->GetComponent<RigidbodyComponent>().localAABB.max.z();
            Vector3f spawnPos = tf.GetWorldPosition() + tf.GetForward() * size + 0.1f;
            // 修改名字以区分不同类型的子弹和owner
            std::string name = "bullet_" + std::to_string(rand());
            GameObject *bullet = owner->GetOwnerWorld()->GetPool("bullet").Spawn(name, spawnPos, tf.GetWorldRotation());

            auto &rb = bullet->GetComponent<RigidbodyComponent>();
            rb.velocity = tf.GetForward() * m_bulletVelocity;

            auto &audio = owner->GetComponent<AudioComponent>();
            float randomPitch = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 1.5f;
            float randomVol = 0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.2f;
            audio.Play("Fire", randomVol, randomPitch);

            // world->GetParticleSystem().Spawn(...);
        }
    }
    else
        m_fireTimer = 0.14f;
}