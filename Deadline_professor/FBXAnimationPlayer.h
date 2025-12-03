#pragma once
#include "TotalHeader.h"
#include "ResourceManager.h"

// 전방 선언 (ResourceManager.h에 정의되어 있지만 명시적으로 선언)
struct FBXAnimation;
struct FBXAnimChannel;
struct FBXKeyframe;

// ============================================
// FBX 애니메이션 플레이어
// ============================================
class FBXAnimationPlayer
{
public:
	FBXAnimationPlayer();
	~FBXAnimationPlayer() = default;

	// 복사/이동 생성자 (기본값 사용)
	FBXAnimationPlayer(const FBXAnimationPlayer&) = delete;
	FBXAnimationPlayer& operator=(const FBXAnimationPlayer&) = delete;
	FBXAnimationPlayer(FBXAnimationPlayer&&) noexcept = default;
	FBXAnimationPlayer& operator=(FBXAnimationPlayer&&) noexcept = default;

	// 초기화
	void Init(const FBXModel* model);

	// 애니메이션 제어
	void PlayAnimation(int animIndex);
	void StopAnimation() noexcept;
	void SetLooping(bool loop) noexcept;
	void SetPlaybackSpeed(float speed) noexcept;

	// 업데이트
	void Update(float deltaTime);

	// 상태 접근
	const std::vector<glm::mat4>& GetBoneTransforms() const noexcept { return finalBoneTransforms; }
	bool IsPlaying() const noexcept { return isPlaying; }
	int GetCurrentAnimationIndex() const noexcept { return currentAnimIndex; }
	float GetCurrentTime() const noexcept { return currentTime; }

private:
	// 보간 함수들
	glm::vec3 InterpolatePosition(const FBXAnimChannel& channel, float time) const;
	glm::quat InterpolateRotation(const FBXAnimChannel& channel, float time) const;
	glm::vec3 InterpolateScale(const FBXAnimChannel& channel, float time) const;

	// 본 변환 계산
	void CalculateBoneTransforms(const FBXAnimation& animation, float time);
	void CalculateBoneHierarchy(int boneIndex, const glm::mat4& parentTransform);

	// 키프레임 찾기 (이진 탐색 최적화 가능)
	int FindKeyframeIndex(const std::vector<FBXKeyframe>& keyframes, float time) const;

private:
	const FBXModel* fbxModel{ nullptr };

	// 애니메이션 상태
	int currentAnimIndex{ -1 };
	float currentTime{ 0.0f };
	bool isPlaying{ false };
	bool looping{ true };
	float playbackSpeed{ 1.0f };

	// 변환 행렬 (캐시)
	std::vector<glm::mat4> boneTransforms;       // 로컬 변환
	std::vector<glm::mat4> finalBoneTransforms;  // 최종 변환 (셰이더에 전달)
};
