#pragma once
#include "TotalHeader.h"
#include "Object.h"

class Camera;

class Player : public Object
{
public:
	Player();
	~Player();

	void Init(Camera*);
	void Update(float) override;

	void MoveForward(float);
	void MoveBackward(float);
	void MoveLeft(float);
	void MoveRight(float);

	void SetMoveSpeed(float);
	float GetMoveSpeed() const;

	void SetCamera(Camera*);
	Camera* GetCamera() const;

	void GetBoundingBox(glm::vec3& outMin, glm::vec3& outMax) const override;

	// 손 모델 렌더링
	void DrawHands(class Renderer* renderer) const;

	// 손 모델 리소스 ID 설정
	void SetLeftHandResourceID(const std::string& id);
	void SetRightHandResourceID(const std::string& id);

private:
	bool TryMove(const glm::vec3& newPos);

	void SyncCameraPosition();

	// 충돌 처리 (Object의 GetBoundingBox를 오버라이드)

	Camera* camera{ nullptr };
	float moveSpeed{ 1.4f }; // 기본값: 걷기 속도 (m/s) - GameConstants::PLAYER_WALK_SPEED 사용

	// 손 모델 리소스 ID
	std::string leftHandResourceID;
	std::string rightHandResourceID;
	std::string leftHandTextureID;
	std::string rightHandTextureID;

};
