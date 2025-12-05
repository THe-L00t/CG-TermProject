#include "Professor.h"
#include "GameConstants.h"

Professor::Professor()
{
	// FBX 원본 비율에 보정값만 곱해서 사용
	SetScale(glm::vec3(GameConstants::PROFESSOR_MODEL_SCALE));

	// 기본 이동 속도 및 감지 범위 설정
	moveSpeed = GameConstants::PROFESSOR_MOVE_SPEED;
	detectionRange = GameConstants::PROFESSOR_DETECTION_RANGE;
}

Professor::Professor(const std::string& meshKey, const std::string& animKey)
	: meshKey(meshKey), animationKey(animKey)
{
	// FBX 원본 비율에 보정값만 곱해서 사용
	SetScale(glm::vec3(GameConstants::PROFESSOR_MODEL_SCALE));

	// 기본 이동 속도 및 감지 범위 설정
	moveSpeed = GameConstants::PROFESSOR_MOVE_SPEED;
	detectionRange = GameConstants::PROFESSOR_DETECTION_RANGE;
}

Professor::Professor(const std::string& meshKey, const std::string& animKey, float width, float height, float depth)
	: meshKey(meshKey), animationKey(animKey), size(width, height, depth)
{
	// FBX 원본 비율에 보정값만 곱해서 사용 (width, height, depth는 정보용으로만 유지)
	SetScale(glm::vec3(GameConstants::PROFESSOR_MODEL_SCALE));

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
	// 크기 변경 시에도 FBX 원본 비율에 보정값만 적용 (size는 정보용으로만 유지)
	SetScale(glm::vec3(GameConstants::PROFESSOR_MODEL_SCALE));
}

glm::vec3 Professor::GetSize() const
{
	return size;
}

void Professor::FleeFromPlayer(float deltaTime)
{
}
