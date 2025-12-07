#include "SoundManager.h"
#include "ResourceManager.h"
#include <iostream>
#include <fmod_errors.h>

// ----------------- Init -----------------
bool SoundManager::Init()
{
    FMOD_RESULT result;

    result = FMOD::System_Create(&system);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD::System_Create failed: " << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    result = system->init(512, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK)
    {
        std::cerr << "FMOD system init failed: " << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    return true;
}

// ----------------- Update -----------------
void SoundManager::Update()
{
    if (system)
        system->update();
}

// ----------------- Play -----------------
void SoundManager::Play(const std::string& name, float volume)
{
    if (!resourceManager) return;
    FMOD::Sound* sound = resourceManager->GetSound(name);
    if (!sound) return;

    // 이전 채널 있으면 강제로 stop 후 제거
    auto it = channels.find(name);
    if (it != channels.end() && it->second)
    {
        it->second->stop();
        channels.erase(it);
    }

    FMOD::Channel* channel = nullptr;
    system->playSound(sound, nullptr, false, &channel);
    if (channel)
    {
        channel->setVolume(volume);
        channels[name] = channel;
    }
}

// ----------------- Stop -----------------
void SoundManager::Stop(const std::string& name)
{
    auto it = channels.find(name);
    if (it != channels.end() && it->second)
    {
        it->second->stop();
        channels.erase(it);
    }
}

// ----------------- Release -----------------
void SoundManager::Release()
{
    // 채널은 FMOD가 자동 관리, 해제할 필요 없음
    if (system)
    {
        system->release();
        system = nullptr;
    }
}

// ----------------- Destructor -----------------
SoundManager::~SoundManager()
{
    Release();
}

FMOD::Channel* SoundManager::GetChannel(const std::string& name)
{
    auto it = channels.find(name);
    if (it != channels.end())
        return it->second;
    return nullptr;
}

// ----------------- IsPlaying -----------------
bool SoundManager::IsPlaying(const std::string& name)
{
    FMOD::Channel* channel = GetChannel(name);
    if (!channel)
        return false;

    bool playing = false;
    FMOD_RESULT result = channel->isPlaying(&playing);
    if (result != FMOD_OK)
    {
        std::cerr << "Failed to get playing state for sound: " << name
            << " Error: " << FMOD_ErrorString(result) << std::endl;
        return false;
    }
    return playing;
}