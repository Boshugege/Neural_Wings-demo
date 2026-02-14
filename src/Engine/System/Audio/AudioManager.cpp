#include "AudioManager.h"
#include "Engine/Core/Components/Components.h"
#include "Engine/System/Resource/ResourceManager.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/GameWorld.h"

AudioManager::~AudioManager()
{
    ClearOneShots();
}
void AudioManager::Update(GameWorld &world, const mCamera &camera)
{
    auto entities = world.GetEntitiesWith<AudioComponent, TransformComponent>();
    for (auto *entity : entities)
    {
        auto &audioComp = entity->GetComponent<AudioComponent>();
        auto &tf = entity->GetComponent<TransformComponent>();
        Vector3f sourcePos = tf.GetWorldPosition();

        for (auto &[name, clip] : audioComp.audioClips)
        {
            if (!clip.isPlaying)
                continue;
            if (clip.is3D)
                Spatialize(clip, sourcePos, camera);
            else
            {
                SetSoundVolume(clip.sound, clip.baseVolume);
                SetSoundPan(clip.sound, 0.5f);
            }

            bool actuallyPlaying = false;
            if (clip.isMulti)
            {
                for (auto &s : clip.aliases)
                {
                    if (IsSoundPlaying(s))
                    {
                        actuallyPlaying = true;
                        break;
                    }
                }
            }
            else
                actuallyPlaying = IsSoundPlaying(clip.sound);

            if (clip.isLooping && !actuallyPlaying)
            {
                PlaySound(clip.isMulti ? clip.aliases[0] : clip.sound);
                actuallyPlaying = true;
            }

            if (!actuallyPlaying)
            {
                clip.isPlaying = false;
            }
        }
    }

    // 独立音效
    for (auto it = m_oneShots.begin(); it != m_oneShots.end();)
    {
        if (!IsSoundPlaying(it->alias))
        {
            UnloadSoundAlias(it->alias);
            it = m_oneShots.erase(it);
            continue;
        }

        float dist = (it->position - camera.Position()).Length();
        float vol = 0.0f;
        if (dist < it->minDis)
            vol = it->baseVolume;
        else if (dist < it->maxDis)
        {
            vol = it->baseVolume * (1.0f - (dist - it->minDis) / (it->maxDis - it->minDis));
        }
        SetSoundVolume(it->alias, vol);

        Vector3f dirToSource = (it->position - camera.Position()).Normalized();
        float pan = dirToSource * camera.Right();
        SetSoundPan(it->alias, (pan + 1.0f) * 0.5f);

        ++it;
    }
}
void AudioManager::Spatialize(AudioClip &clip, const Vector3f &sourcePos, const mCamera &listener)
{
    Vector3f listenerPos = listener.Position();
    Vector3f diff = sourcePos - listenerPos;
    float dist = diff.Length();

    float finalVolume = 0.0f;
    if (dist < clip.minDis)
    {
        finalVolume = clip.baseVolume;
    }
    else if (dist < clip.maxDis)
    {
        finalVolume = clip.baseVolume * (1.0f - (dist - clip.minDis) / (clip.maxDis - clip.minDis));
    }
    SetSoundVolume(clip.sound, finalVolume);

    if (dist > 0.001f)
    {
        Vector3f dirToSource = diff.Normalized();
        Vector3f listenerRight = listener.Right();

        float dot = dirToSource * listenerRight;
        float pan = (dot + 1.0f) * 0.5f;
        SetSoundPan(clip.sound, pan);
        if (clip.isMulti)
        {
            for (auto &s : clip.aliases)
            {
                SetSoundVolume(s, finalVolume);
                SetSoundPan(s, pan);
            }
        }
    }
}

void AudioManager::ClearOneShots()
{
    int count = 0;
    for (auto &s : m_oneShots)
    {
        if (s.alias.frameCount > 0)
        {
            StopSound(s.alias);
            UnloadSoundAlias(s.alias);
            count++;
        }
    }
    m_oneShots.clear();

    if (count > 0)
    {
        std::cout << "[AudioManager]: Cleared " << count << " spatial one-shot aliases." << std::endl;
    }
}

#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
using json = nlohmann::json;
void AudioManager::LoadLibrary(const std::string &jsonPath)
{
    std::ifstream file(jsonPath);
    if (!file.is_open())
        return;

    json data = json::parse(file);
    if (data.contains("sfx"))
    {
        for (auto &[name, path] : data["sfx"].items())
        {
            m_sfxLibrary[name] = path;
            m_resourceManager.GetSound(path);
        }
    }
    if (data.contains("music"))
    {
        for (auto &[name, path] : data["music"].items())
        {
            m_musicLibrary[name] = path;
        }
    }
}

void AudioManager::Play2D(const std::string &name, float volume, float pitch)
{
    auto it = m_sfxLibrary.find(name);
    if (it != m_sfxLibrary.end())
    {
        Sound s = m_resourceManager.GetSound(it->second);
        if (s.frameCount > 0)
        {
            SetSoundVolume(s, volume);
            SetSoundPitch(s, pitch);
            PlaySound(s);
        }
        else
        {
            std::cerr << "[AudioManager]: Sound alias '" << name << "' not found!" << std::endl;
        }
    }
}

void AudioManager::PlayMusic(const std::string &name, float volume)
{
    auto it = m_musicLibrary.find(name);
    if (it != m_musicLibrary.end())
    {
        Music m = m_resourceManager.GetMusic(it->second);
        SetMusicVolume(m, volume);
        PlayMusicStream(m);
    }
}

void AudioManager::PlaySpatial(const std::string &name, Vector3f position, float minDis, float maxDis, float volume, float pitch)
{
    auto it = m_sfxLibrary.find(name);
    if (it == m_sfxLibrary.end())
        return;
    Sound original = m_resourceManager.GetSound(it->second);
    if (original.frameCount == 0)
        return;

    SpatialOneShot s;
    s.alias = LoadSoundAlias(original);
    s.position = position;
    s.baseVolume = volume;
    s.minDis = minDis;
    s.maxDis = maxDis;
    PlaySound(s.alias);
    m_oneShots.push_back(s);
}
