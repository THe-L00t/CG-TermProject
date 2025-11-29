#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp, float fov, float aspect)
	: position(pos), direction(target), worldUp(worldUp), fov(glm::radians(fov)), aspect(aspect)
{
	up = this->worldUp;
	moveSpd = 5.0f;
	dirSpd = 0.1f;
	zoomSpd = 2.0f;

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
