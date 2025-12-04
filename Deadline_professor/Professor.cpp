#include "Professor.h"

Professor::Professor()
{
}

Professor::Professor(const std::string& meshKey, const std::string& animKey)
	: meshKey(meshKey), animationKey(animKey)
{
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

void Professor::FleeFromPlayer(float deltaTime)
{
}
