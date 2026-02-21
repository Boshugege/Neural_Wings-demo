#include "Engine/Graphics/Particle/IParticleInitializer.h"
#include "Engine/Math/Math.h"
#include "Engine/Utils/JsonParser.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
;
class CollisionInit : public IParticleInitializer
{
public:
    Vector3f spawnPos;
    Vector3f normal;
    Vector3f relVel;
    Vector2f size;
    float minSpeed;
    float maxSpeed;
    float maxLife;
    float tau;
    float impulse;
    int burstCount = 0;
    void LoadConfig(const json &config) override
    {
        spawnPos = JsonParser::ToVector3f(config["spawnPos"]);
        normal = JsonParser::ToVector3f(config["normal"]);
        relVel = JsonParser::ToVector3f(config["relVel"]);
        size = JsonParser::ToVector2f(config["size"]);
        minSpeed = (float)config["minSpeed"];
        maxSpeed = (float)config["maxSpeed"];
        tau = (float)config["tau"];
        maxLife = (float)config["maxLife"];
        impulse = config["impulse"];
        burstCount = impulse * ((float)config["countFactor"]);
    }
    virtual int BurstCount() override
    {
        std::cout << "Collision Burst Count: " << burstCount << std::endl;
        return burstCount;
    };
    float SmoothSpeed(float impluse)
    {
        float t = 1.0f - exp(-impluse / tau); // 使用指数函数实现平滑过渡
        return minSpeed + t * (maxSpeed - minSpeed);
    }
    void Initialize(std::vector<GPUParticle> &gpuParticles, size_t start, size_t count) override
    {
        for (size_t i = start; i < start + count; ++i)
        {
            if (i >= gpuParticles.size())
                break;
            auto &p = gpuParticles[i];
            p.position = Vector3f::ZERO; // 在碰撞点的坐标系
            p.velocity = Vector3f::RandomCycle(normal, SmoothSpeed(impulse) * ((float)(std::rand()) / RAND_MAX));
            p.acceleration = Vector3f::ZERO;
            p.color = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
            p.size = size / (p.velocity.Length() < 1.0f ? 1.0f : p.velocity.Length());
            p.life = Vector2f(0.5f * maxLife / p.velocity.Length());
            p.rotation = p.velocity.Length();
            p.randomID = std::rand() % 1000;
            p.ID = i;
        }
    }
};
