#pragma once
#include "Object.h"
#include "AIController.h"
#include "GameConstants.h"

class PathFinder;

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

	// AI 시스템
	void SetAIController(AIController* aiController);
	AIController* GetAIController() const;
	void SetPatrolTarget(const glm::vec3& targetPos);

	// PathFinder 설정 (NavMesh 기반 경로 탐색)
	void SetPathFinder(PathFinder* pathFinder);
	PathFinder* GetPathFinder() const;

	// ⭐⭐⭐ 충돌 감지 함수 추가
	bool IsCollidingWithPlayer(float collisionRadius = GameConstants::PROFESSOR_COLLISION_RADIUS) const;

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

	// AI 시스템
	AIController* aiController{ nullptr };
	PathFinder* pathFinder{ nullptr };  // ⭐ NavMesh 기반 경로 탐색
	glm::vec3 patrolTarget{ 0.0f };

	// ⭐ 플레이어 반대 방향으로 탈출 목표 계산
	glm::vec3 CalculateEscapeTarget(const glm::vec3& npcPos, const glm::vec3& playerPos);

};
