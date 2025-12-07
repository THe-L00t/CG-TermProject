#pragma once
#include <fmod.hpp>
#include <string>
#include <unordered_map>

class ResourceManager;

class SoundManager
{
public:
    SoundManager() = default;
    ~SoundManager();

    // 시스템 초기화 / 업데이트 / 해제
    bool Init();
    void Update();
    void Release();

    // ResourceManager 연결
    void SetResourceManager(ResourceManager* rm) { resourceManager = rm; }

    // 사운드 재생 / 정지
    void Play(const std::string& name, float volume = 1.0f);
    void Stop(const std::string& name);

    // FMOD 시스템 접근 (ResourceManager에서 사운드 로드 시 필요)
    FMOD::System* GetSystem() { return system; }
    FMOD::Channel* GetChannel(const std::string& name);

    bool IsPlaying(const std::string& name);

private:
    FMOD::System* system = nullptr;
    ResourceManager* resourceManager = nullptr;

    // 재생 중인 채널 관리
    std::unordered_map<std::string, FMOD::Channel*> channels;
};
