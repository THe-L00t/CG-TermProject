#include "Camera.h"

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
