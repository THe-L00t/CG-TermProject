#include "Professor.h"
#include "GameConstants.h"

Professor::Professor()
{
	// 기본 크기 사용 + FBX 모델 보정값 적용
	SetScale(size * GameConstants::PROFESSOR_MODEL_SCALE);

	// 기본 이동 속도 및 감지 범위 설정
	moveSpeed = GameConstants::PROFESSOR_MOVE_SPEED;
	detectionRange = GameConstants::PROFESSOR_DETECTION_RANGE;
}

Professor::Professor(const std::string& meshKey, const std::string& animKey)
	: meshKey(meshKey), animationKey(animKey)
{
	// 기본 크기 사용 + FBX 모델 보정값 적용
	SetScale(size * GameConstants::PROFESSOR_MODEL_SCALE);

	// 기본 이동 속도 및 감지 범위 설정
	moveSpeed = GameConstants::PROFESSOR_MOVE_SPEED;
	detectionRange = GameConstants::PROFESSOR_DETECTION_RANGE;
}

Professor::Professor(const std::string& meshKey, const std::string& animKey, float width, float height, float depth)
	: meshKey(meshKey), animationKey(animKey), size(width, height, depth)
{
	// 지정된 크기 사용 + FBX 모델 보정값 적용
	SetScale(size * GameConstants::PROFESSOR_MODEL_SCALE);

	// 기본 이동 속도 및 감지 범위 설정
	moveSpeed = GameConstants::PROFESSOR_MOVE_SPEED;
	detectionRange = GameConstants::PROFESSOR_DETECTION_RANGE;
}

Professor::~Professor()
{
}

void Professor::Update(float deltaTime)
{
	Object::Update(deltaTime);

	if (playerRef != nullptr)
	{
		playerPosition = playerRef->GetPosition();
	}

	FleeFromPlayer(deltaTime);
}

void Professor::SetMeshKey(const std::string& key)
{
	meshKey = key;
}

void Professor::SetAnimationKey(const std::string& key)
{
	animationKey = key;
}

std::string Professor::GetMeshKey() const
{
	return meshKey;
}

std::string Professor::GetAnimationKey() const
{
	return animationKey;
}

void Professor::SetPlayerPosition(const glm::vec3& playerPos)
{
	playerPosition = playerPos;
}

void Professor::SetPlayerReference(Object* player)
{
	playerRef = player;
}

void Professor::SetMoveSpeed(float speed)
{
	moveSpeed = speed;
}

float Professor::GetMoveSpeed() const
{
	return moveSpeed;
}

void Professor::SetDetectionRange(float range)
{
	detectionRange = range;
}

float Professor::GetDetectionRange() const
{
	return detectionRange;
}

void Professor::SetDirection(const glm::vec3& dir)
{
	direction = dir;
}

glm::vec3 Professor::GetDirection() const
{
	return direction;
}

void Professor::SetSize(float width, float height, float depth)
{
	size = glm::vec3(width, height, depth);
	// 크기 변경 시 스케일도 함께 업데이트 (보정값 적용)
	SetScale(size * GameConstants::PROFESSOR_MODEL_SCALE);
}

glm::vec3 Professor::GetSize() const
{
	return size;
}

void Professor::FleeFromPlayer(float deltaTime)
{
}
