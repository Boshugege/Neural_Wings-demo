#pragma once
#include "Engine/Graphics/Camera/mCamera.h"
#include "Engine/Core/Components/AudioComponent.h"
#include <unordered_map>
#include <string>
class ResourceManager;
class GameWorld;

struct SpatialOneShot
{
    Sound alias;
    Vector3f position;
    float baseVolume;
    float minDis;
    float maxDis;
    bool finished = false;
};

class AudioManager
{
public:
    AudioManager(ResourceManager &rm) : m_resourceManager(rm) {}
    void LoadLibrary(const std::string &jsonPath);

    ~AudioManager();

    void Update(GameWorld &world, const mCamera &camera);

    void Play2D(const std::string &path, float volume = 1.0f, float pitch = 1.0f);
    void PlayMusic(const std::string &name, float volume = 0.5f);
    void PlaySpatial(const std::string &name, Vector3f position, float minDis = 5.0f, float maxDis = 50.0f, float volume = 1.0f, float pitch = 1.0f);

    void ClearOneShots();

private:
    ResourceManager &m_resourceManager;
    std::unordered_map<std::string, std::string> m_sfxLibrary;
    std::unordered_map<std::string, std::string> m_musicLibrary;

    std::vector<SpatialOneShot> m_oneShots;

    void Spatialize(AudioClip &clip, const Vector3f &sourceWorldPos, const mCamera &listener);
};