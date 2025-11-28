#include "AnimationPlayer.h"

AnimationPlayer::AnimationPlayer() : meshData(nullptr)
{
}

AnimationPlayer::~AnimationPlayer()
{
}

void AnimationPlayer::SetMesh(const XMeshData* mesh)
{
	meshData = mesh;
	if (meshData && meshData->has_skeleton) {
		animState.bone_transforms.resize(meshData->bones.size());
		animState.final_transforms.resize(meshData->bones.size());

		// 기본 포즈로 초기화
		for (size_t i = 0; i < meshData->bones.size(); ++i) {
			animState.bone_transforms[i] = meshData->bones[i].local_transform;
			animState.final_transforms[i] = glm::mat4(1.0f);
		}
	}
}

bool AnimationPlayer::PlayAnimation(const std::string& animName, bool loop)
{
	if (!meshData || meshData->animations.empty()) {
		std::cerr << "No animations available" << std::endl;
		return false;
	}

	// 이름으로 애니메이션 찾기
	for (const auto& anim : meshData->animations) {
		if (std::string(anim.name) == animName) {
			animState.current_clip = &anim;
			animState.current_time = 0.0f;
			animState.is_playing = true;
			animState.is_looping = loop;
			std::cout << "Playing animation: " << animName << std::endl;
			return true;
		}
	}

	std::cerr << "Animation '" << animName << "' not found" << std::endl;
	return false;
}

bool AnimationPlayer::PlayAnimation(size_t animIndex, bool loop)
{
	if (!meshData || animIndex >= meshData->animations.size()) {
		std::cerr << "Invalid animation index" << std::endl;
		return false;
	}

	animState.current_clip = &meshData->animations[animIndex];
	animState.current_time = 0.0f;
	animState.is_playing = true;
	animState.is_looping = loop;

	std::cout << "Playing animation: " << animState.current_clip->name << std::endl;
	return true;
}

void AnimationPlayer::Stop()
{
	animState.is_playing = false;
	animState.current_time = 0.0f;
}

void AnimationPlayer::Pause()
{
	animState.is_playing = false;
}

void AnimationPlayer::Resume()
{
	if (animState.current_clip) {
		animState.is_playing = true;
	}
}

void AnimationPlayer::SetPlaybackSpeed(float speed)
{
	animState.playback_speed = speed;
}

void AnimationPlayer::Update(float deltaTime)
{
	if (!animState.is_playing || !animState.current_clip) {
		return;
	}

	// 시간 업데이트
	animState.current_time += deltaTime * animState.playback_speed * animState.current_clip->ticks_per_second;

	// 루프 처리
	if (animState.current_time >= animState.current_clip->duration) {
		if (animState.is_looping) {
			animState.current_time = fmod(animState.current_time, animState.current_clip->duration);
		}
		else {
			animState.current_time = animState.current_clip->duration;
			animState.is_playing = false;
		}
	}

	// 본 변환 계산
	CalculateBoneTransforms();
}

glm::vec3 AnimationPlayer::InterpolatePosition(const AnimationTrack& track, float time)
{
	if (track.keyframes.size() == 1) {
		return track.keyframes[0].position;
	}

	int index = FindKeyframeIndex(track, time);
	if (index < 0 || index >= static_cast<int>(track.keyframes.size()) - 1) {
		return track.keyframes.back().position;
	}

	const AnimationKeyframe& key0 = track.keyframes[index];
	const AnimationKeyframe& key1 = track.keyframes[index + 1];

	float t = (time - key0.timestamp) / (key1.timestamp - key0.timestamp);
	return glm::mix(key0.position, key1.position, t);
}

glm::quat AnimationPlayer::InterpolateRotation(const AnimationTrack& track, float time)
{
	if (track.keyframes.size() == 1) {
		return track.keyframes[0].rotation;
	}

	int index = FindKeyframeIndex(track, time);
	if (index < 0 || index >= static_cast<int>(track.keyframes.size()) - 1) {
		return track.keyframes.back().rotation;
	}

	const AnimationKeyframe& key0 = track.keyframes[index];
	const AnimationKeyframe& key1 = track.keyframes[index + 1];

	float t = (time - key0.timestamp) / (key1.timestamp - key0.timestamp);
	return glm::slerp(key0.rotation, key1.rotation, t);
}

glm::vec3 AnimationPlayer::InterpolateScale(const AnimationTrack& track, float time)
{
	if (track.keyframes.size() == 1) {
		return track.keyframes[0].scale;
	}

	int index = FindKeyframeIndex(track, time);
	if (index < 0 || index >= static_cast<int>(track.keyframes.size()) - 1) {
		return track.keyframes.back().scale;
	}

	const AnimationKeyframe& key0 = track.keyframes[index];
	const AnimationKeyframe& key1 = track.keyframes[index + 1];

	float t = (time - key0.timestamp) / (key1.timestamp - key0.timestamp);
	return glm::mix(key0.scale, key1.scale, t);
}

int AnimationPlayer::FindKeyframeIndex(const AnimationTrack& track, float time)
{
	for (size_t i = 0; i < track.keyframes.size() - 1; ++i) {
		if (time < track.keyframes[i + 1].timestamp) {
			return static_cast<int>(i);
		}
	}
	return static_cast<int>(track.keyframes.size()) - 2;
}

void AnimationPlayer::CalculateBoneTransforms()
{
	if (!meshData || !meshData->has_skeleton || !animState.current_clip) {
		return;
	}

	// 각 트랙의 현재 변환 계산
	std::vector<glm::mat4> localTransforms(meshData->bones.size());

	// 기본 로컬 변환으로 초기화
	for (size_t i = 0; i < meshData->bones.size(); ++i) {
		localTransforms[i] = meshData->bones[i].local_transform;
	}

	// 애니메이션 트랙 적용
	for (const auto& track : animState.current_clip->tracks) {
		if (track.bone_index >= meshData->bones.size()) {
			continue;
		}

		// 현재 시간의 변환 보간
		glm::vec3 position = InterpolatePosition(track, animState.current_time);
		glm::quat rotation = InterpolateRotation(track, animState.current_time);
		glm::vec3 scale = InterpolateScale(track, animState.current_time);

		// 로컬 변환 행렬 생성
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, position);
		transform = transform * glm::mat4_cast(rotation);
		transform = glm::scale(transform, scale);

		localTransforms[track.bone_index] = transform;
	}

	// 계층 구조에 따라 최종 변환 계산
	for (size_t i = 0; i < meshData->bones.size(); ++i) {
		if (meshData->bones[i].parent_index == -1) {
			// 루트 본
			CalculateBoneTransform(static_cast<int>(i), glm::mat4(1.0f));
		}
	}
}

void AnimationPlayer::CalculateBoneTransform(int boneIndex, const glm::mat4& parentTransform)
{
	if (boneIndex < 0 || boneIndex >= static_cast<int>(meshData->bones.size())) {
		return;
	}

	// 애니메이션 트랙에서 로컬 변환 가져오기
	glm::mat4 localTransform = meshData->bones[boneIndex].local_transform;

	// 애니메이션이 재생 중이면 보간된 변환 사용
	if (animState.current_clip) {
		for (const auto& track : animState.current_clip->tracks) {
			if (track.bone_index == static_cast<uint32_t>(boneIndex)) {
				glm::vec3 position = InterpolatePosition(track, animState.current_time);
				glm::quat rotation = InterpolateRotation(track, animState.current_time);
				glm::vec3 scale = InterpolateScale(track, animState.current_time);

				localTransform = glm::mat4(1.0f);
				localTransform = glm::translate(localTransform, position);
				localTransform = localTransform * glm::mat4_cast(rotation);
				localTransform = glm::scale(localTransform, scale);
				break;
			}
		}
	}

	// 월드 변환 계산
	glm::mat4 worldTransform = parentTransform * localTransform;
	animState.bone_transforms[boneIndex] = worldTransform;

	// 최종 변환 = 월드 변환 * 오프셋 행렬
	animState.final_transforms[boneIndex] = worldTransform * meshData->bones[boneIndex].offset_matrix;

	// 자식 본들 재귀적으로 처리
	for (size_t i = 0; i < meshData->bones.size(); ++i) {
		if (meshData->bones[i].parent_index == boneIndex) {
			CalculateBoneTransform(static_cast<int>(i), worldTransform);
		}
	}
}
