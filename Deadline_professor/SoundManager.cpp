#include "SoundManager.h"
#include <iostream>

SoundManager::~SoundManager()
{
    Release();
}

bool SoundManager::Init()
{
    FMOD::System_Create(&system);
    system->init(512, FMOD_INIT_NORMAL, nullptr);
    return true;
}

void SoundManager::Update()
{
    if (system) system->update();
}

bool SoundManager::LoadSound(const std::string& name, const std::string& path, bool loop, bool is3D)
{
    if (sounds.find(name) != sounds.end())
        return true;

    FMOD_MODE mode = FMOD_DEFAULT;

    mode |= is3D ? FMOD_3D : FMOD_2D;
    mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

    FMOD::Sound* sound = nullptr;
    FMOD_RESULT res = system->createSound(path.c_str(), mode, nullptr, &sound);

    if (res != FMOD_OK)
    {
        std::cerr << "FMOD Load Error: " << FMOD_ErrorString(res) << std::endl;
        return false;
    }

    sounds[name] = sound;
    return true;
}

void SoundManager::Play(const std::string& name, float volume)
{
    if (sounds.find(name) == sounds.end())
        return;

    FMOD::Channel* channel = nullptr;
    system->playSound(sounds[name], nullptr, false, &channel);

    if (channel)
    {
        channel->setVolume(volume);
        channels[name] = channel;
    }
}

void SoundManager::Stop(const std::string& name)
{
    if (channels.find(name) != channels.end())
    {
        channels[name]->stop();
    }
}

void SoundManager::Release()
{
    for (auto& s : sounds)
        s.second->release();

    if (system)
    {
        system->close();
        system->release();
    }
}
