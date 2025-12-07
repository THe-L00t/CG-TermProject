#pragma once

#include "TotalHeader.h"
#include "PathFinder.h"

class PathFinder;

// ========================================
// AI 캐릭터 제어 시스템
// ========================================
class AIController
{
public:
	AIController(PathFinder* pathFinder);
	~AIController();

	// 행동 모드
	enum class BehaviorMode
	{
		IDLE,           // 대기 중
		CHASING,        // 플레이어 추적 중
		PATROLLING,     // 순찰 중
		STUCK           // 길을 잃음
	};

	// 목표 설정
	void SetTargetPosition(const glm::vec3& targetPos);
	void ClearTarget();

	// 경로 기반 이동
	void UpdateMovement(float deltaTime);
	glm::vec3 GetNextMoveDirection() const;

	// 상태 관리
	BehaviorMode GetBehaviorMode() const { return behaviorMode; }
	const Path& GetCurrentPath() const { return currentPath; }
	bool HasReachedTarget() const;

	// 플레이어 추적 (최대 거리 기반)
	void ChaseTarget(const glm::vec3& targetPos, float maxChaseDistance, float deltaTime);

	// 경로 유효성 재확인
	void ValidateAndUpdatePath(const glm::vec3& currentPos);

private:
	PathFinder* pathFinder = nullptr;
	Path currentPath;
	BehaviorMode behaviorMode = BehaviorMode::IDLE;
	glm::vec3 targetPosition = glm::vec3(0.0f);
	bool hasTarget = false;

	// 경로 이동 진행도
	float moveProgress = 0.0f;
};
