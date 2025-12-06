#include "Player.h"
#include "Camera.h"
#include "GameConstants.h"
#include "Engine.h"
#include "CollisionManager.h"

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
	if (!camera) return;

	// 카메라가 바라보는 방향으로 이동
	glm::vec3 forward = glm::normalize(camera->GetDirection() - camera->GetPosition());
	forward.y = 0.0f; // Y축 이동 방지 (평면 이동만)
	if (glm::length(forward) > 0.001f) {
		forward = glm::normalize(forward);
	}

	glm::vec3 newPos = position + forward * moveSpeed * deltaTime;
	TryMove(newPos);
}

void Player::MoveBackward(float deltaTime)
{
	if (!camera) return;

	glm::vec3 forward = glm::normalize(camera->GetDirection() - camera->GetPosition());
	forward.y = 0.0f;
	if (glm::length(forward) > 0.001f) {
		forward = glm::normalize(forward);
	}

	glm::vec3 newPos = position - forward * moveSpeed * deltaTime;
	TryMove(newPos);
}

void Player::MoveLeft(float deltaTime)
{
	if (!camera) return;

	glm::vec3 forward = glm::normalize(camera->GetDirection() - camera->GetPosition());
	forward.y = 0.0f;
	if (glm::length(forward) > 0.001f) {
		forward = glm::normalize(forward);
	}

	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 newPos = position - right * moveSpeed * deltaTime;
	TryMove(newPos);
}

void Player::MoveRight(float deltaTime)
{
	if (!camera) return;

	glm::vec3 forward = glm::normalize(camera->GetDirection() - camera->GetPosition());
	forward.y = 0.0f;
	if (glm::length(forward) > 0.001f) {
		forward = glm::normalize(forward);
	}

	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 newPos = position + right * moveSpeed * deltaTime;
	TryMove(newPos);
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
		// 이전 카메라 위치와 direction 저장 (방향 유지를 위해)
		glm::vec3 oldCameraPos = camera->GetPosition();
		glm::vec3 oldDirection = camera->GetDirection();
		glm::vec3 viewVector = oldDirection - oldCameraPos; // 바라보는 방향 벡터

		// 카메라 위치를 플레이어 눈 높이로 설정
		glm::vec3 newCameraPos = position;
		newCameraPos.y += GameConstants::PLAYER_EYE_HEIGHT;
		camera->SetPosition(newCameraPos);

		// 카메라가 바라보는 방향 유지 (1인칭 시점)
		// 이동 전 바라보던 방향 벡터를 유지하면서 새 위치에서의 direction 계산
		if (glm::length(viewVector) > 0.01f) {
			// 이전 방향 벡터를 유지
			glm::vec3 newDirection = newCameraPos + viewVector;
			camera->SetDirection(newDirection);
		} else {
			// 방향이 설정되지 않았으면 앞쪽(-Z)을 바라보도록 설정
			camera->SetDirection(newCameraPos + glm::vec3(0.0f, 0.0f, -5.0f));
		}
	}
}

void Player::GetBoundingBox(glm::vec3& outMin, glm::vec3& outMax) const
{
	// 플레이어 충돌 박스: 벽의 1/3 크기, 높이는 카메라 높이까지
	float collisionWidth = GameConstants::TILE_SIZE / 3.0f;
	float collisionDepth = GameConstants::TILE_SIZE / 3.0f;
	float collisionHeight = GameConstants::PLAYER_EYE_HEIGHT;

	glm::vec3 halfSize(collisionWidth * 0.5f, collisionHeight * 0.5f, collisionDepth * 0.5f);

	// 바운딩 박스 중심을 플레이어 중심 높이로 설정
	glm::vec3 center = position;
	center.y += collisionHeight * 0.5f;

	outMin = center - halfSize;
	outMax = center + halfSize;
}

bool Player::TryMove(const glm::vec3& newPos)
{
	// CollisionManager를 통해 충돌 검사
	extern Engine* g_engine;
	if (g_engine)
	{
		CollisionManager* collisionMgr = g_engine->GetCollisionManager();
		if (collisionMgr)
		{
			// 충돌 검사 - 충돌이 없으면 이동
			if (!collisionMgr->CheckCollisionAt(this, newPos))
			{
				SetPosition(newPos);
				SyncCameraPosition();
				return true;
			}
			// 충돌이 있으면 이동하지 않음
			return false;
		}
	}

	// CollisionManager가 없으면 무조건 이동
	SetPosition(newPos);
	SyncCameraPosition();
	return true;
}
