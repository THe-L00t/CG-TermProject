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
	//glm::vec3 forward = glm::normalize(direction - position);
	//right = glm::normalize(glm::cross(forward, worldUp));
	//up = glm::normalize(glm::cross(right, forward));

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

void Camera::MoveForward(float deltaTime)
{
	glm::vec3 forward = glm::normalize(direction - position);
	position += forward * moveSpd * deltaTime;
	direction += forward * moveSpd * deltaTime;
	UpdateVectors();

	std::cout << "DEBUG : Camera moved forward to position " 
		<< position.x << ", " << position.y << ", " << position.z << std::endl;
}

void Camera::MoveBackward(float deltaTime)
{
	glm::vec3 forward = glm::normalize(direction - position);
	position -= forward * moveSpd * deltaTime;
	direction -= forward * moveSpd * deltaTime;
	UpdateVectors();

	std::cout << "DEBUG : Camera moved backward to position " 
		<< position.x << ", " << position.y << ", " << position.z << std::endl;
}

void Camera::MoveLeft(float deltaTime)
{
	position -= right * moveSpd * deltaTime;
	direction -= right * moveSpd * deltaTime;
	UpdateVectors();

	std::cout << "DEBUG : Camera moved left to position " 
		<< position.x << ", " << position.y << ", " << position.z << std::endl;
}

void Camera::MoveRight(float deltaTime)
{
	position += right * moveSpd * deltaTime;
	direction += right * moveSpd * deltaTime;
	UpdateVectors();

	std::cout << "DEBUG : Camera moved right to position " 
		<< position.x << ", " << position.y << ", " << position.z << std::endl;
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
