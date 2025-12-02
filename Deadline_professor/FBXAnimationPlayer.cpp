#include "FBXAnimationPlayer.h"
#include "ResourceManager.h"
#include <gl/glm/gtc/type_ptr.hpp>
#include <gl/glm/gtc/quaternion.hpp>

FBXAnimationPlayer::FBXAnimationPlayer()
    : fbxModel(nullptr), currentAnimIndex(-1), currentTime(0.0f),
      isPlaying(false), looping(true), playbackSpeed(1.0f)
{
}

FBXAnimationPlayer::~FBXAnimationPlayer()
{
}

void FBXAnimationPlayer::Init(const FBXModel* model)
{
    fbxModel = model;

    if (fbxModel && !fbxModel->bones.empty()) {
        boneTransforms.resize(fbxModel->bones.size(), glm::mat4(1.0f));
        finalBoneTransforms.resize(fbxModel->bones.size(), glm::mat4(1.0f));
    }

    currentAnimIndex = -1;
    currentTime = 0.0f;
    isPlaying = false;
}

void FBXAnimationPlayer::PlayAnimation(int animIndex)
{
    if (!fbxModel || animIndex < 0 || animIndex >= static_cast<int>(fbxModel->animations.size())) {
        return;
    }

    currentAnimIndex = animIndex;
    currentTime = 0.0f;
    isPlaying = true;
}

void FBXAnimationPlayer::StopAnimation()
{
    isPlaying = false;
}

void FBXAnimationPlayer::SetLooping(bool loop)
{
    looping = loop;
}

void FBXAnimationPlayer::SetPlaybackSpeed(float speed)
{
    playbackSpeed = speed;
}

void FBXAnimationPlayer::Update(float deltaTime)
{
    if (!isPlaying || !fbxModel || currentAnimIndex < 0) {
        return;
    }

    const FBXAnimation& animation = fbxModel->animations[currentAnimIndex];

    currentTime += deltaTime * playbackSpeed;

    if (currentTime > animation.duration) {
        if (looping) {
            currentTime = std::fmod(currentTime, animation.duration);
        } else {
            currentTime = animation.duration;
            isPlaying = false;
        }
    }

    CalculateBoneTransforms(animation, currentTime);
}

void FBXAnimationPlayer::CalculateBoneTransforms(const FBXAnimation& animation, float time)
{
    for (const auto& channel : animation.channels) {
        auto it = fbxModel->boneMap.find(channel.boneName);
        if (it == fbxModel->boneMap.end()) {
            continue;
        }

        int boneIndex = it->second;

        glm::vec3 position = InterpolatePosition(channel, time);
        glm::quat rotation = InterpolateRotation(channel, time);
        glm::vec3 scale = InterpolateScale(channel, time);

        glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotationMat = glm::mat4(rotation);
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

        boneTransforms[boneIndex] = translationMat * rotationMat * scaleMat;
    }

    for (size_t i = 0; i < fbxModel->bones.size(); ++i) {
        if (fbxModel->bones[i].parentIndex == -1) {
            CalculateBoneHierarchy(static_cast<int>(i), fbxModel->globalInverseTransform);
        }
    }
}

void FBXAnimationPlayer::CalculateBoneHierarchy(int boneIndex, const glm::mat4& parentTransform)
{
    const FBXBone& bone = fbxModel->bones[boneIndex];

    glm::mat4 globalTransform = parentTransform * boneTransforms[boneIndex];

    finalBoneTransforms[boneIndex] = globalTransform * bone.offsetMatrix;

    for (size_t i = 0; i < fbxModel->bones.size(); ++i) {
        if (fbxModel->bones[i].parentIndex == boneIndex) {
            CalculateBoneHierarchy(static_cast<int>(i), globalTransform);
        }
    }
}

glm::vec3 FBXAnimationPlayer::InterpolatePosition(const FBXAnimChannel& channel, float time)
{
    if (channel.keyframes.empty()) {
        return glm::vec3(0.0f);
    }

    if (channel.keyframes.size() == 1) {
        return channel.keyframes[0].position;
    }

    int index = FindPositionKeyframe(channel, time);
    int nextIndex = (index + 1) % static_cast<int>(channel.keyframes.size());

    const FBXKeyframe& key1 = channel.keyframes[index];
    const FBXKeyframe& key2 = channel.keyframes[nextIndex];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.position;
    }

    float factor = (time - key1.time) / deltaTime;
    factor = std::min(std::max(factor, 0.0f), 1.0f);

    return glm::mix(key1.position, key2.position, factor);
}

glm::quat FBXAnimationPlayer::InterpolateRotation(const FBXAnimChannel& channel, float time)
{
    if (channel.keyframes.empty()) {
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }

    if (channel.keyframes.size() == 1) {
        return channel.keyframes[0].rotation;
    }

    int index = FindRotationKeyframe(channel, time);
    int nextIndex = (index + 1) % static_cast<int>(channel.keyframes.size());

    const FBXKeyframe& key1 = channel.keyframes[index];
    const FBXKeyframe& key2 = channel.keyframes[nextIndex];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.rotation;
    }

    float factor = (time - key1.time) / deltaTime;
    factor = std::min(std::max(factor, 0.0f), 1.0f);

    return glm::slerp(key1.rotation, key2.rotation, factor);
}

glm::vec3 FBXAnimationPlayer::InterpolateScale(const FBXAnimChannel& channel, float time)
{
    if (channel.keyframes.empty()) {
        return glm::vec3(1.0f);
    }

    if (channel.keyframes.size() == 1) {
        return channel.keyframes[0].scale;
    }

    int index = FindScaleKeyframe(channel, time);
    int nextIndex = (index + 1) % static_cast<int>(channel.keyframes.size());

    const FBXKeyframe& key1 = channel.keyframes[index];
    const FBXKeyframe& key2 = channel.keyframes[nextIndex];

    float deltaTime = key2.time - key1.time;
    if (deltaTime <= 0.0f) {
        return key1.scale;
    }

    float factor = (time - key1.time) / deltaTime;
    factor = std::min(std::max(factor, 0.0f), 1.0f);

    return glm::mix(key1.scale, key2.scale, factor);
}

int FBXAnimationPlayer::FindPositionKeyframe(const FBXAnimChannel& channel, float time)
{
    for (size_t i = 0; i < channel.keyframes.size() - 1; ++i) {
        if (time < channel.keyframes[i + 1].time) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(channel.keyframes.size()) - 1;
}

int FBXAnimationPlayer::FindRotationKeyframe(const FBXAnimChannel& channel, float time)
{
    for (size_t i = 0; i < channel.keyframes.size() - 1; ++i) {
        if (time < channel.keyframes[i + 1].time) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(channel.keyframes.size()) - 1;
}

int FBXAnimationPlayer::FindScaleKeyframe(const FBXAnimChannel& channel, float time)
{
    for (size_t i = 0; i < channel.keyframes.size() - 1; ++i) {
        if (time < channel.keyframes[i + 1].time) {
            return static_cast<int>(i);
        }
    }
    return static_cast<int>(channel.keyframes.size()) - 1;
}
