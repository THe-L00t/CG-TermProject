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

	void SyncCameraPosition();

private:
	Camera* camera{nullptr};
	float moveSpeed{5.f};
};
