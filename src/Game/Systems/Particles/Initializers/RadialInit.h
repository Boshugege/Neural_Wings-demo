#include "Engine/Graphics/ParticleSystem/IParticleInitializer.h"
class RadialInit : public IParticleInitializer
{
public:
    float minSpeed = 5.0f;
    float maxSpeed = 10.0f;

    void Initialize(std::vector<GPUParticle> &particles, const TransformComponent &tf, int simSpace) override
    {
        // TODO:随体系与世界系区别
        for (auto &p : particles)
        {
            Vector3f randomDir = Vector3f::RandomSphere();
            float speed = static_cast<float>(rand()) / RAND_MAX * (maxSpeed - minSpeed) + minSpeed;
            p.velocity = randomDir * speed;
            p.position = tf.position;      // TODO:加入偏离
            p.life = Vector2f(2.0f, 2.0f); // 2s寿命
            p.randomID = rand() % 10000;
            p.color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
            p.size = Vector2f(1.0f, 1.0f);
        }
    }
};
