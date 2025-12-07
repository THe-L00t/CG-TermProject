#pragma once
#include <fmod.hpp>
#include <string>
#include <unordered_map>

class SoundManager
{
public:
    SoundManager() = default;
    ~SoundManager();

    bool Init();
    void Update();
    void Release();

    bool LoadSound(const std::string& name, const std::string& path, bool loop = false, bool is3D = false);
    void Play(const std::string& name, float volume = 1.0f);
    void Stop(const std::string& name);

private:
    FMOD::System* system = nullptr;
    std::unordered_map<std::string, FMOD::Sound*> sounds;
    std::unordered_map<std::string, FMOD::Channel*> channels;
};
