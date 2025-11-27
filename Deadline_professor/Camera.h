#pragma once
#include "TotalHeader.h"
class Camera
{
public:
	// 생성자 코딩하기

	glm::mat4 GetViewMat() const;
	glm::mat4 GetProjMat() const;
	glm::mat4 GetOrthMat(float, float, float, float) const;

private:
	// LookAt 용
	glm::vec3 position{};
	glm::vec3 direction{};
	glm::vec3 up{};

	// 캐싱 및 재계산용
	glm::vec3 right;
	glm::vec3 worldUp;

	// Perspective 용
	float fov{45.f};
	float aspect{16.f/9.f};
	float n{0.1f};
	float f{100.f};

	// 속도 관련
	float moveSpd;
	float dirSpd;
	float zoomSpd;


};

