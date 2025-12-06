#include "Camera.h"
#include "GameConstants.h"

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp, float fov, float aspect)
	: position(pos), direction(target), worldUp(worldUp), fov(glm::radians(fov)), aspect(aspect)
{
	up = this->worldUp;

	// 현실적인 카메라 설정 적용
	moveSpd = GameConstants::PLAYER_WALK_SPEED;     // 플레이어 걷기 속도와 동일
	dirSpd = GameConstants::CAMERA_SENSITIVITY;     // 마우스 감도
	zoomSpd = 2.0f;

	// Near/Far plane 설정
	n = GameConstants::CAMERA_NEAR_PLANE;
	f = GameConstants::CAMERA_FAR_PLANE;

	// right 벡터 계산
	glm::vec3 forward = glm::normalize(direction - position);
	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
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

void Camera::MoveForward(float deltaTime)
{
	glm::vec3 forward = glm::normalize(direction - position);
	position += forward * moveSpd * deltaTime;
	direction += forward * moveSpd * deltaTime;
}

void Camera::MoveBackward(float deltaTime)
{
	glm::vec3 forward = glm::normalize(direction - position);
	position -= forward * moveSpd * deltaTime;
	direction -= forward * moveSpd * deltaTime;
}

void Camera::MoveLeft(float deltaTime)
{
	position -= right * moveSpd * deltaTime;
	direction -= right * moveSpd * deltaTime;
}

void Camera::MoveRight(float deltaTime)
{
	position += right * moveSpd * deltaTime;
	direction += right * moveSpd * deltaTime;
}

void Camera::MoveUp(float deltaTime)
{
	position += worldUp * moveSpd * deltaTime;
	direction += worldUp * moveSpd * deltaTime;
}

void Camera::MoveDown(float deltaTime)
{
	position -= worldUp * moveSpd * deltaTime;
	direction -= worldUp * moveSpd * deltaTime;
}

void Camera::Rotate(float yawDelta, float pitchDelta)
{
	// Get current forward direction
	glm::vec3 forward = glm::normalize(direction - position);

	// Apply yaw rotation (around world up axis)
	glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yawDelta * dirSpd, worldUp);
	forward = glm::vec3(yawRotation * glm::vec4(forward, 0.0f));

	// Apply pitch rotation (around right axis)
	glm::mat4 pitchRotation = glm::rotate(glm::mat4(1.0f), pitchDelta * dirSpd, right);
	forward = glm::vec3(pitchRotation * glm::vec4(forward, 0.0f));

	// Update direction to maintain distance from position
	float distance = glm::length(direction - position);
	direction = position + forward * distance;

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
