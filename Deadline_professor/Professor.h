#pragma once
#include "Object.h"

class Professor : public Object
{
public:
	Professor();
	Professor(const std::string& meshKey, const std::string& animKey);
	~Professor();

	void Update(float deltaTime) override;

	void SetMeshKey(const std::string& key);
	void SetAnimationKey(const std::string& key);

	std::string GetMeshKey() const;
	std::string GetAnimationKey() const;

	void SetPlayerPosition(const glm::vec3& playerPos);
	void SetPlayerReference(Object* player);

	void SetMoveSpeed(float speed);
	float GetMoveSpeed() const;

	void SetDetectionRange(float range);
	float GetDetectionRange() const;

	void SetDirection(const glm::vec3& dir);
	glm::vec3 GetDirection() const;

private:
	void FleeFromPlayer(float deltaTime);

	std::string meshKey{};
	std::string animationKey{};

	glm::vec3 playerPosition{};
	glm::vec3 direction{};

	Object* playerRef{nullptr};

	float moveSpeed{5.0f};
	float detectionRange{10.0f};
};
