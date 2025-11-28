#pragma once
#include "TotalHeader.h"
#include "ResourceManager.h"

class AnimationPlayer
{
public:
	AnimationPlayer();
	~AnimationPlayer();

	// 애니메이션 제어
	void SetMesh(const XMeshData* mesh);
	bool PlayAnimation(const std::string& animName, bool loop = true);
	bool PlayAnimation(size_t animIndex, bool loop = true);
	void Stop();
	void Pause();
	void Resume();
	void SetPlaybackSpeed(float speed);

	// 본 인덱스 조회
	int GetBoneIndex(const std::string& boneName) const;

	// 업데이트 (deltaTime: 초 단위)
	void Update(float deltaTime);

	// 본 변환 행렬 가져오기
	const std::vector<glm::mat4>& GetFinalTransforms() const { return animState.final_transforms; }
	bool IsPlaying() const { return animState.is_playing; }
	float GetCurrentTime() const { return animState.current_time; }

private:
	const XMeshData* meshData;
	AnimationState animState;

	// 키프레임 보간
	glm::vec3 InterpolatePosition(const AnimationTrack& track, float time);
	glm::quat InterpolateRotation(const AnimationTrack& track, float time);
	glm::vec3 InterpolateScale(const AnimationTrack& track, float time);

	// 키프레임 인덱스 찾기
	int FindKeyframeIndex(const AnimationTrack& track, float time);

	// 계층 구조 변환 계산
	void CalculateBoneTransforms();
	void CalculateBoneTransform(int boneIndex, const glm::mat4& parentTransform);
};
