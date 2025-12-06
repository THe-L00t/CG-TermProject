#pragma once
#include "Object.h"

class Professor : public Object
{
public:
	Professor();
	Professor(const std::string& meshKey, const std::string& animKey);
	Professor(const std::string& meshKey, const std::string& animKey, float width, float height, float depth);
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

	void SetSize(float width, float height, float depth);
	glm::vec3 GetSize() const;

private:
	void FleeFromPlayer(float deltaTime);

	std::string meshKey{};
	std::string animationKey{};

	glm::vec3 playerPosition{};
	glm::vec3 direction{};

	Object* playerRef{nullptr};

	glm::vec3 size{1.0f, 1.8f, 1.0f};  // 기본 크기 (width, height, depth) in meters

	float moveSpeed{5.5f};           // 기본값: 이동 속도 (m/s) - GameConstants::PROFESSOR_MOVE_SPEED 사용
	float detectionRange{15.0f};     // 기본값: 감지 범위 (m) - GameConstants::PROFESSOR_DETECTION_RANGE 사용
};
