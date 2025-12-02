#pragma once
#include "TotalHeader.h"

struct FBXModel;
struct FBXAnimation;
struct FBXAnimChannel;
struct FBXKeyframe;

class FBXAnimationPlayer
{
public:
    FBXAnimationPlayer();
    ~FBXAnimationPlayer();

    void Init(const FBXModel* model);

    void PlayAnimation(int animIndex);
    void StopAnimation();
    void SetLooping(bool loop);
    void SetPlaybackSpeed(float speed);

    void Update(float deltaTime);

    const std::vector<glm::mat4>& GetBoneTransforms() const { return finalBoneTransforms; }

    bool IsPlaying() const { return isPlaying; }
    int GetCurrentAnimationIndex() const { return currentAnimIndex; }
    float GetCurrentTime() const { return currentTime; }

private:
    glm::vec3 InterpolatePosition(const FBXAnimChannel& channel, float time);
    glm::quat InterpolateRotation(const FBXAnimChannel& channel, float time);
    glm::vec3 InterpolateScale(const FBXAnimChannel& channel, float time);

    void CalculateBoneTransforms(const FBXAnimation& animation, float time);
    void CalculateBoneHierarchy(int boneIndex, const glm::mat4& parentTransform);

    int FindPositionKeyframe(const FBXAnimChannel& channel, float time);
    int FindRotationKeyframe(const FBXAnimChannel& channel, float time);
    int FindScaleKeyframe(const FBXAnimChannel& channel, float time);

private:
    const FBXModel* fbxModel;

    int currentAnimIndex;
    float currentTime;
    bool isPlaying;
    bool looping;
    float playbackSpeed;

    std::vector<glm::mat4> boneTransforms;
    std::vector<glm::mat4> finalBoneTransforms;
};
