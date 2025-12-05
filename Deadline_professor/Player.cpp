#include "Player.h"
#include "Camera.h"
#include "GameConstants.h"

Player::Player()
{
	// 현실적인 스케일 적용 (1 unit = 1 meter)
	SetScale(glm::vec3(GameConstants::PLAYER_WIDTH,
	                   GameConstants::PLAYER_HEIGHT,
	                   GameConstants::PLAYER_DEPTH));

	// 기본 이동 속도 설정 (걷기)
	moveSpeed = GameConstants::PLAYER_WALK_SPEED;
}

Player::~Player()
{
}

void Player::Init(Camera* cam)
{
	camera = cam;
	SyncCameraPosition();
}

void Player::Update(float deltaTime)
{
	Object::Update(deltaTime);
	SyncCameraPosition();
}

void Player::MoveForward(float deltaTime)
{
	glm::vec3 newPos = position;
	newPos.z -= moveSpeed * deltaTime;
	SetPosition(newPos);
	SyncCameraPosition();
}

void Player::MoveBackward(float deltaTime)
{
	glm::vec3 newPos = position;
	newPos.z += moveSpeed * deltaTime;
	SetPosition(newPos);
	SyncCameraPosition();
}

void Player::MoveLeft(float deltaTime)
{
	glm::vec3 newPos = position;
	newPos.x -= moveSpeed * deltaTime;
	SetPosition(newPos);
	SyncCameraPosition();
}

void Player::MoveRight(float deltaTime)
{
	glm::vec3 newPos = position;
	newPos.x += moveSpeed * deltaTime;
	SetPosition(newPos);
	SyncCameraPosition();
}

void Player::SetMoveSpeed(float speed)
{
	moveSpeed = speed;
}

float Player::GetMoveSpeed() const
{
	return moveSpeed;
}

void Player::SetCamera(Camera* cam)
{
	camera = cam;
	SyncCameraPosition();
}

Camera* Player::GetCamera() const
{
	return camera;
}

void Player::SyncCameraPosition()
{
	if (camera)
	{
		// 카메라 위치를 플레이어 위치와 동기화
		// Camera 클래스에 SetPosition 메서드가 있다면 사용
		// position은 Object 클래스의 protected 멤버
	}
}
