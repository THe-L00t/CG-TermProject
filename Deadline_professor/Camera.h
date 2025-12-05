#pragma once
#include "TotalHeader.h"
class Camera
{
public:
	Camera(glm::vec3 pos = glm::vec3(0.0f, 1.6f, 3.0f),  // 기본 높이: 플레이어 눈 높이 (1.6m)
		   glm::vec3 target = glm::vec3(0.0f, 1.6f, 0.0f),
		   glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
		   float fov = 60.0f,  // GameConstants::CAMERA_FOV 사용
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
	float fov{60.f};      // 기본값: 시야각 60도 - GameConstants::CAMERA_FOV 사용
	float aspect{16.f/9.f};
	float n{0.1f};        // 기본값: Near plane - GameConstants::CAMERA_NEAR_PLANE 사용
	float f{100.f};       // 기본값: Far plane - GameConstants::CAMERA_FAR_PLANE 사용

	// 속도 관련
	float moveSpd;
	float dirSpd;
	float zoomSpd;


};

