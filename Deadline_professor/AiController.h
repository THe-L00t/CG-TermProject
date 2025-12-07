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
	//void ChaseTarget(const glm::vec3& currentPos, const glm::vec3& targetPos, float maxChaseDistance, float deltaTime);

	// 경로 유효성 재확인
	void ValidateAndUpdatePath(const glm::vec3& currentPos);

	// NPC 현재 위치 설정 (경로 계산에 필요)
	void SetCurrentPosition(const glm::vec3& pos) { currentPosition = pos; }
	glm::vec3 GetCurrentPosition() const { return currentPosition; }

	// ⭐ 다음 waypoint 방향 가져오기 (회전 예측용)
	glm::vec3 GetUpcomingMoveDirection() const;
	// ⭐ 다음 waypoint까지의 남은 거리
	float GetDistanceToNextWaypoint() const;

private:
	PathFinder* pathFinder = nullptr;
	glm::vec3 currentPosition = glm::vec3(0.0f);  // NPC의 현재 위치
	Path currentPath;
	BehaviorMode behaviorMode = BehaviorMode::IDLE;
	glm::vec3 targetPosition = glm::vec3(0.0f);
	bool hasTarget = false;

	// 경로 이동 진행도
	float moveProgress = 0.0f;
	float lastPathUpdateTime = 0.0f;  // 경로 갱신 주기 관리
	const float PATH_UPDATE_INTERVAL = 0.5f;  // 0.5초마다 경로 검증
	const float WAYPOINT_REACH_DISTANCE = 0.5f;  // waypoint 도달 거리
};
