#include "FBXAnimationPlayer.h"
#include "ResourceManager.h"
#include <gl/glm/gtc/type_ptr.hpp>
#include <gl/glm/gtc/quaternion.hpp>
#include <algorithm>

// ============================================
// 생성자 / 초기화
// ============================================
FBXAnimationPlayer::FBXAnimationPlayer()
	: fbxModel(nullptr), currentAnimIndex(-1), currentTime(0.0f),
	  isPlaying(false), looping(true), playbackSpeed(1.0f)
{
}

void FBXAnimationPlayer::Init(const FBXModel* model)
{
	fbxModel = model;

	if (fbxModel && !fbxModel->bones.empty()) {
		size_t boneCount = fbxModel->bones.size();
		boneTransforms.resize(boneCount, glm::mat4(1.0f));
		finalBoneTransforms.resize(boneCount, glm::mat4(1.0f));
	}

	currentAnimIndex = -1;
	currentTime = 0.0f;
	isPlaying = false;
}

// ============================================
// 애니메이션 제어
// ============================================
void FBXAnimationPlayer::PlayAnimation(int animIndex)
{
	if (!fbxModel || animIndex < 0 || animIndex >= static_cast<int>(fbxModel->animations.size())) {
		return;
	}

	currentAnimIndex = animIndex;
	currentTime = 0.0f;
	isPlaying = true;
}

void FBXAnimationPlayer::StopAnimation() noexcept
{
	isPlaying = false;
}

void FBXAnimationPlayer::SetLooping(bool loop) noexcept
{
	looping = loop;
}

void FBXAnimationPlayer::SetPlaybackSpeed(float speed) noexcept
{
	playbackSpeed = speed;
}

// ============================================
// 업데이트
// ============================================
void FBXAnimationPlayer::Update(float deltaTime)
{
	if (!isPlaying || !fbxModel || currentAnimIndex < 0) {
		return;
	}

	const FBXAnimation& animation = fbxModel->animations[currentAnimIndex];

	// 시간 업데이트
	currentTime += deltaTime * playbackSpeed;

	// 루핑 처리
	if (currentTime > animation.duration) {
		if (looping) {
			currentTime = std::fmod(currentTime, animation.duration);
		}
		else {
			currentTime = animation.duration;
			isPlaying = false;
		}
	}

	// 본 변환 계산
	CalculateBoneTransforms(animation, currentTime);
}

// ============================================
// 본 변환 계산
// ============================================
void FBXAnimationPlayer::CalculateBoneTransforms(const FBXAnimation& animation, float time)
{
	// 모든 본을 bind pose로 초기화
	for (size_t i = 0; i < boneTransforms.size(); ++i) {
		boneTransforms[i] = fbxModel->bones[i].nodeTransform;
	}

	// 애니메이션 채널의 변환 적용
	for (const auto& channel : animation.channels) {
		auto it = fbxModel->boneMap.find(channel.boneName);
		if (it == fbxModel->boneMap.end()) {
			continue;
		}

		int boneIndex = it->second;

		// 애니메이션 데이터 보간
		glm::vec3 position = InterpolatePosition(channel, time);
		glm::quat rotation = InterpolateRotation(channel, time);
		glm::vec3 scale = InterpolateScale(channel, time);

		// TRS 조합
		glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationMat = glm::mat4_cast(rotation);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

		boneTransforms[boneIndex] = translationMat * rotationMat * scaleMat;
	}

	// 루트 본부터 계층 구조 계산
	for (size_t i = 0; i < fbxModel->bones.size(); ++i) {
		if (fbxModel->bones[i].parentIndex == -1) {
			CalculateBoneHierarchy(static_cast<int>(i), glm::mat4(1.0f));
		}
	}
}

void FBXAnimationPlayer::CalculateBoneHierarchy(int boneIndex, const glm::mat4& parentTransform)
{
	const FBXBone& bone = fbxModel->bones[boneIndex];

	// 글로벌 변환 = 부모 변환 * 현재 본의 로컬 변환
	glm::mat4 globalTransform = parentTransform * boneTransforms[boneIndex];

	// 최종 변환 = globalInverseTransform * globalTransform * offsetMatrix
	finalBoneTransforms[boneIndex] = fbxModel->globalInverseTransform * globalTransform * bone.offsetMatrix;

	// 자식 본들 재귀 처리
	for (size_t i = 0; i < fbxModel->bones.size(); ++i) {
		if (fbxModel->bones[i].parentIndex == boneIndex) {
			CalculateBoneHierarchy(static_cast<int>(i), globalTransform);
		}
	}
}

// ============================================
// 보간 함수들
// ============================================
int FBXAnimationPlayer::FindKeyframeIndex(const std::vector<FBXKeyframe>& keyframes, float time) const
{
	if (keyframes.empty()) return -1;
	if (keyframes.size() == 1) return 0;

	// 선형 탐색 (TODO: 이진 탐색으로 최적화 가능)
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		if (time < keyframes[i + 1].time) {
			return static_cast<int>(i);
		}
	}
	return static_cast<int>(keyframes.size()) - 1;
}

glm::vec3 FBXAnimationPlayer::InterpolatePosition(const FBXAnimChannel& channel, float time) const
{
	const auto& keyframes = channel.keyframes;
	if (keyframes.empty()) return glm::vec3(0.0f);
	if (keyframes.size() == 1) return keyframes[0].position;

	int index = FindKeyframeIndex(keyframes, time);
	if (index >= static_cast<int>(keyframes.size()) - 1) {
		return keyframes[index].position;
	}

	const FBXKeyframe& key1 = keyframes[index];
	const FBXKeyframe& key2 = keyframes[index + 1];

	float deltaTime = key2.time - key1.time;
	if (deltaTime <= 0.0001f) {
		return key1.position;
	}

	float factor = std::clamp((time - key1.time) / deltaTime, 0.0f, 1.0f);
	return glm::mix(key1.position, key2.position, factor);
}

glm::quat FBXAnimationPlayer::InterpolateRotation(const FBXAnimChannel& channel, float time) const
{
	const auto& keyframes = channel.keyframes;
	if (keyframes.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (keyframes.size() == 1) return keyframes[0].rotation;

	int index = FindKeyframeIndex(keyframes, time);
	if (index >= static_cast<int>(keyframes.size()) - 1) {
		return keyframes[index].rotation;
	}

	const FBXKeyframe& key1 = keyframes[index];
	const FBXKeyframe& key2 = keyframes[index + 1];

	float deltaTime = key2.time - key1.time;
	if (deltaTime <= 0.0001f) {
		return key1.rotation;
	}

	float factor = std::clamp((time - key1.time) / deltaTime, 0.0f, 1.0f);
	return glm::slerp(key1.rotation, key2.rotation, factor);
}

glm::vec3 FBXAnimationPlayer::InterpolateScale(const FBXAnimChannel& channel, float time) const
{
	const auto& keyframes = channel.keyframes;
	if (keyframes.empty()) return glm::vec3(1.0f);
	if (keyframes.size() == 1) return keyframes[0].scale;

	int index = FindKeyframeIndex(keyframes, time);
	if (index >= static_cast<int>(keyframes.size()) - 1) {
		return keyframes[index].scale;
	}

	const FBXKeyframe& key1 = keyframes[index];
	const FBXKeyframe& key2 = keyframes[index + 1];

	float deltaTime = key2.time - key1.time;
	if (deltaTime <= 0.0001f) {
		return key1.scale;
	}

	float factor = std::clamp((time - key1.time) / deltaTime, 0.0f, 1.0f);
	return glm::mix(key1.scale, key2.scale, factor);
}
