#pragma once
#include "IComponent.h"
#include "raylib.h"
#include <unordered_map>
#include <string>
#include <iostream>

struct AudioClip
{
    Sound sound;

    bool is3D = true;
    bool isLooping = false;
    float baseVolume = 1.0f;
    float basePitch = 1.0f;

    float minDis = 5.0f;  // 最大音量半径
    float maxDis = 50.0f; // 最小音量半径

    bool isPlaying = false;

    AudioClip() : sound({0})
    {
    }
    ~AudioClip()
    {
        Cleanup();
    }
    AudioClip(const AudioClip &) = delete;
    AudioClip &operator=(const AudioClip &) = delete;

    AudioClip(AudioClip &&other) noexcept
        : sound(other.sound), is3D(other.is3D), isLooping(other.isLooping),
          baseVolume(other.baseVolume), basePitch(other.basePitch),
          minDis(other.minDis), maxDis(other.maxDis), isPlaying(other.isPlaying),
          aliases(std::move(other.aliases)), nextAliasIndex(other.nextAliasIndex),
          isMulti(other.isMulti)
    {
        other.sound = {0};
        other.isMulti = false;
    }

    AudioClip &operator=(AudioClip &&other) noexcept
    {
        if (this != &other)
        {
            this->Cleanup();

            sound = other.sound;
            aliases = std::move(other.aliases);
            is3D = other.is3D;
            isLooping = other.isLooping;
            baseVolume = other.baseVolume;
            basePitch = other.basePitch;
            minDis = other.minDis;
            maxDis = other.maxDis;
            isPlaying = other.isPlaying;
            nextAliasIndex = other.nextAliasIndex;
            isMulti = other.isMulti;

            other.sound = {0};
        }
        return *this;
    }

    void Cleanup()
    {
        for (auto &s : aliases)
        {
            if (s.frameCount > 0 && s.stream.buffer != nullptr)
            {
                UnloadSoundAlias(s);
            }
        }
        aliases.clear();
    }

    // 多重播放
    std::vector<Sound> aliases;
    int nextAliasIndex = 0;
    bool isMulti = false;

    void SetupMultiVoice(int voiceCount = 10)
    {
        isMulti = true;
        aliases.clear();
        for (int i = 0; i < voiceCount; ++i)
        {
            aliases.push_back(LoadSoundAlias(sound));
        }
    }
};

class AudioComponent : public IComponent
{
public:
    AudioComponent() = default;
    ~AudioComponent() = default;
    std::unordered_map<std::string, AudioClip> audioClips;

    void Play(const std::string &name, float vol = 1.0f, float pitch = 1.0f)
    {
        if (!audioClips.count(name))
        {
            std::cerr << "[AudioComponent]: No audio clip named '" << name << "' found!" << std::endl;
            return;
        }
        auto &clip = audioClips[name];

        Sound target;
        if (clip.isMulti && !clip.aliases.empty())
        {
            target = clip.aliases[clip.nextAliasIndex];
            clip.nextAliasIndex = (clip.nextAliasIndex + 1) % clip.aliases.size();
        }
        else
        {
            target = clip.sound;
        }

        if (target.frameCount == 0)
        {
            std::cerr << "[AudioComponent]: Sound '" << name << "' is not loaded properly!" << std::endl;
            return;
        }

        SetSoundVolume(target, clip.baseVolume * vol);
        SetSoundPitch(target, clip.basePitch * pitch);

        PlaySound(target);
        clip.isPlaying = true;
    }
    void Stop(const std::string &name)
    {
        if (audioClips.count(name))
        {
            auto &clip = audioClips[name];
            StopSound(clip.sound);
            if (clip.isMulti)
            {
                for (auto &s : clip.aliases)
                    StopSound(s);
            }
            clip.isPlaying = false;
        }
    }

    void SetVolume(const std::string &name, float vol)
    {
        if (audioClips.count(name))
            audioClips[name].baseVolume = vol;
    }

    void SetPitch(const std::string &name, float pitch)
    {
        if (audioClips.count(name))
        {
            audioClips[name].basePitch = pitch;
            SetSoundPitch(audioClips[name].sound, pitch);
        }
    }
};