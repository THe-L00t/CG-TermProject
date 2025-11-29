#pragma once
#include "TotalHeader.h"
class Camera
{
public:
	Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f),
		   glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f),
		   glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
		   float fov = 45.0f,
		   float aspect = 16.0f / 9.0f);

	glm::mat4 GetViewMat() const;
	glm::mat4 GetProjMat() const;
	glm::mat4 GetOrthMat(float, float, float, float) const;

	// 카메라 제어
	void MoveForward(float deltaTime);
	void MoveBackward(float deltaTime);
	void MoveLeft(float deltaTime);
	void MoveRight(float deltaTime);
	void MoveUp(float deltaTime);
	void MoveDown(float deltaTime);
	void Rotate(float yaw, float pitch); // 각도 단위: 라디안
	void Zoom(float delta);

	// Getters
	glm::vec3 GetPosition() const { return position; }
	glm::vec3 GetDirection() const { return direction; }
	float GetMoveSpeed() const { return moveSpd; }
	float GetRotateSpeed() const { return dirSpd; }

	// Setters
	void SetMoveSpeed(float speed) { moveSpd = speed; }
	void SetRotateSpeed(float speed) { dirSpd = speed; }
	void SetPosition(const glm::vec3& pos) { position = pos; UpdateVectors(); }

private:
	void UpdateVectors();
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

