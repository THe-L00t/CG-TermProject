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
