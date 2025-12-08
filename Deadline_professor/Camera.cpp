#include "Camera.h"
#include "GameConstants.h"

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp, float fov, float aspect)
	: position(pos), direction(target), worldUp(worldUp), fov(glm::radians(fov)), aspect(aspect),
	targetPosition(pos), targetDirection(target)
{
	up = this->worldUp;

	// 현실적인 카메라 설정 적용
	moveSpd = GameConstants::PLAYER_WALK_SPEED;     // 플레이어 걷기 속도와 동일
	dirSpd = GameConstants::CAMERA_SENSITIVITY;     // 마우스 감도
	zoomSpd = 2.0f;

	// Near/Far plane 설정
	n = GameConstants::CAMERA_NEAR_PLANE;
	f = GameConstants::CAMERA_FAR_PLANE;

	UpdateVectors();
}

glm::mat4 Camera::GetViewMat() const
{
    return glm::lookAt(position, direction, up);
}

glm::mat4 Camera::GetProjMat() const
{
    return glm::perspective(fov, aspect, n, f);
}

glm::mat4 Camera::GetOrthMat(float left, float right, float bottom, float top) const
{
    return glm::ortho(left,right,bottom,top,n,f);
}

void Camera::UpdateVectors()
{
	// Calculate forward direction from position and target
	glm::vec3 forward = glm::normalize(direction - position);

	// Recalculate right and up vectors
	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
}

glm::vec3 Camera::Lerp(const glm::vec3& start, const glm::vec3& end, float t) const
{
	return start + t * (end - start);
}

void Camera::Update(float deltaTime)
{
	if (smoothMode) {
		// NaN/Inf 체크
		if (glm::any(glm::isnan(targetPosition)) || glm::any(glm::isinf(targetPosition))) {
			std::cerr << "[ERROR] Camera::Update() - targetPosition is invalid!" << std::endl;
			return;
		}

		if (glm::any(glm::isnan(targetDirection)) || glm::any(glm::isinf(targetDirection))) {
			std::cerr << "[ERROR] Camera::Update() - targetDirection is invalid!" << std::endl;
			return;
		}

		// lerp 계수 계산 (0~1 사이 값)
		float t = glm::clamp(lerpSpeed * deltaTime, 0.0f, 1.0f);

		// ⭐ position만 lerp하고, direction은 상대적으로 계산
		glm::vec3 oldViewVector = direction - position;
		glm::vec3 targetViewVector = targetDirection - targetPosition;

		position = Lerp(position, targetPosition, t);

		// ⭐ 방향 벡터도 lerp
		glm::vec3 newViewVector = Lerp(oldViewVector, targetViewVector, t);
		direction = position + newViewVector;

		UpdateVectors();
	}
}

void Camera::MoveForward(float deltaTime)
{
	glm::vec3 forward = glm::normalize(direction - position);

	if (smoothMode) {
		// ⭐ 스무스 모드: 목표 위치만 업데이트
		targetPosition += forward * moveSpd * deltaTime;
		targetDirection += forward * moveSpd * deltaTime;
	}
	else {
		// 즉시 이동
		position += forward * moveSpd * deltaTime;
		direction += forward * moveSpd * deltaTime;
		UpdateVectors();
	}
}

void Camera::MoveBackward(float deltaTime)
{
	glm::vec3 forward = glm::normalize(direction - position);

	if (smoothMode) {
		targetPosition -= forward * moveSpd * deltaTime;
		targetDirection -= forward * moveSpd * deltaTime;
	}
	else {
		position -= forward * moveSpd * deltaTime;
		direction -= forward * moveSpd * deltaTime;
		UpdateVectors();
	}
}

void Camera::MoveLeft(float deltaTime)
{
	if (smoothMode) {
		targetPosition -= right * moveSpd * deltaTime;
		targetDirection -= right * moveSpd * deltaTime;
	}
	else {
		position -= right * moveSpd * deltaTime;
		direction -= right * moveSpd * deltaTime;
		UpdateVectors();
	}
}

void Camera::MoveRight(float deltaTime)
{
	if (smoothMode) {
		targetPosition += right * moveSpd * deltaTime;
		targetDirection += right * moveSpd * deltaTime;
	}
	else {
		position += right * moveSpd * deltaTime;
		direction += right * moveSpd * deltaTime;
		UpdateVectors();
	}
}

void Camera::MoveUp(float deltaTime)
{
	if (smoothMode) {
		targetPosition += worldUp * moveSpd * deltaTime;
		targetDirection += worldUp * moveSpd * deltaTime;
	}
	else {
		position += worldUp * moveSpd * deltaTime;
		direction += worldUp * moveSpd * deltaTime;
		UpdateVectors();
	}
}

void Camera::MoveDown(float deltaTime)
{
	if (smoothMode) {
		targetPosition -= worldUp * moveSpd * deltaTime;
		targetDirection -= worldUp * moveSpd * deltaTime;
	}
	else {
		position -= worldUp * moveSpd * deltaTime;
		direction -= worldUp * moveSpd * deltaTime;
		UpdateVectors();
	}
}

void Camera::Rotate(float yawDelta, float pitchDelta)
{
	// ⭐ 현재 실제 위치 기준으로 회전 (targetPosition 아님!)
	glm::vec3 currentPos = position;
	glm::vec3 currentDir = direction;

	// Get current forward direction
	glm::vec3 forward = glm::normalize(currentDir - currentPos);

	// Apply yaw rotation (around world up axis)
	glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yawDelta * dirSpd, worldUp);
	forward = glm::vec3(yawRotation * glm::vec4(forward, 0.0f));

	// Apply pitch rotation (around right axis)
	glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), pitchDelta * dirSpd, right);
	forward = glm::vec3(pitchRotation * glm::vec4(forward, 0.0f));

	// Update direction to maintain distance from position
	float distance = glm::length(currentDir - currentPos);

	// ⭐ 회전은 즉시 반영 (부드럽게 하지 않음)
	direction = position + forward * distance;
	targetDirection = direction;  // ⭐ 목표값도 함께 업데이트

	UpdateVectors();
}

void Camera::Zoom(float delta)
{
	fov -= delta * zoomSpd * 0.01f;
	fov = glm::clamp(fov, glm::radians(1.0f), glm::radians(90.0f));
}

bool Camera::IsBoxInFrustum(const glm::vec3& minBound, const glm::vec3& maxBound) const
{
	// View-Projection 행렬 계산
	glm::mat4 VP = GetProjMat() * GetViewMat();

	// AABB의 8개 꼭짓점
	glm::vec3 corners[8] = {
		glm::vec3(minBound.x, minBound.y, minBound.z),
		glm::vec3(maxBound.x, minBound.y, minBound.z),
		glm::vec3(minBound.x, maxBound.y, minBound.z),
		glm::vec3(maxBound.x, maxBound.y, minBound.z),
		glm::vec3(minBound.x, minBound.y, maxBound.z),
		glm::vec3(maxBound.x, minBound.y, maxBound.z),
		glm::vec3(minBound.x, maxBound.y, maxBound.z),
		glm::vec3(maxBound.x, maxBound.y, maxBound.z)
	};

	// 8개 꼭짓점 중 하나라도 프러스텀 안에 있으면 렌더링
	// 클립 스페이스로 변환 후 NDC 범위 확인
	for (int i = 0; i < 8; ++i) {
		glm::vec4 clip = VP * glm::vec4(corners[i], 1.0f);

		// Perspective divide
		if (clip.w != 0.0f) {
			glm::vec3 ndc = glm::vec3(clip) / clip.w;

			// NDC 범위: [-1, 1]
			if (ndc.x >= -1.0f && ndc.x <= 1.0f &&
				ndc.y >= -1.0f && ndc.y <= 1.0f &&
				ndc.z >= -1.0f && ndc.z <= 1.0f) {
				return true;  // 하나라도 안에 있으면 렌더링
			}
		}
	}

	// AABB가 프러스텀을 완전히 감싸는 경우 체크
	// (보수적으로 렌더링)
	glm::vec3 center = (minBound + maxBound) * 0.5f;
	glm::vec4 centerClip = VP * glm::vec4(center, 1.0f);
	if (centerClip.w != 0.0f) {
		glm::vec3 centerNDC = glm::vec3(centerClip) / centerClip.w;
		float radius = glm::length(maxBound - center);

		// 바운딩 스피어가 프러스텀과 교차하는지 확인
		if (centerNDC.x + radius >= -1.0f && centerNDC.x - radius <= 1.0f &&
			centerNDC.y + radius >= -1.0f && centerNDC.y - radius <= 1.0f &&
			centerNDC.z + radius >= -1.0f && centerNDC.z - radius <= 1.0f) {
			return true;
		}
	}

	return false;  // 프러스텀 밖
}
