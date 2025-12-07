#include "AIController.h"
#include "PathFinder.h"
#include "GameConstants.h"

AIController::AIController(PathFinder* pathFinder)
	: pathFinder(pathFinder)
{
}

AIController::~AIController()
{
}

void AIController::SetTargetPosition(const glm::vec3& targetPos)
{
	targetPosition = targetPos;
	hasTarget = true;
	behaviorMode = BehaviorMode::CHASING;

	if (pathFinder)
	{
		std::cout << "AIController: Target set at (" << targetPos.x << ", " << targetPos.y << ", " << targetPos.z << ")" << std::endl;

		// ⭐ 디버그: 현재 위치 출력
		std::cout << "AIController: Current position at (" << currentPosition.x << ", " << currentPosition.y << ", " << currentPosition.z << ")" << std::endl;

		// 현재 위치에서 목표까지 경로 찾기
		currentPath = pathFinder->FindPath(currentPosition, targetPosition);

		if (currentPath.isValid && !currentPath.waypoints.empty())
		{
			std::cout << "AIController: Path found! " << currentPath.waypoints.size() << " waypoints" << std::endl;
			behaviorMode = BehaviorMode::CHASING;
		}
		else
		{
			std::cout << "AIController: No path found to target. Stuck state." << std::endl;
			behaviorMode = BehaviorMode::STUCK;
		}
	}
}

void AIController::ClearTarget()
{
	hasTarget = false;
	behaviorMode = BehaviorMode::IDLE;
	currentPath.isValid = false;
}

// ========================================
// Phase 1-2: GetNextMoveDirection 구현
// ========================================
glm::vec3 AIController::GetNextMoveDirection() const
{
	// 경로가 유효하지 않거나 waypoint가 없으면 이동하지 않음
	if (!currentPath.isValid || currentPath.waypoints.empty())
	{
		return glm::vec3(0.0f);
	}

	// 현재 waypoint 조회
	glm::vec3 nextWaypoint = currentPath.GetNextWaypoint();
	if (nextWaypoint == glm::vec3(0.0f))
	{
		// 경로가 끝남 (모든 waypoint 통과)
		return glm::vec3(0.0f);
	}

	// 방향 벡터 계산 (다음 waypoint - 현재 위치)
	glm::vec3 direction = nextWaypoint - currentPosition;

	// 거리 확인
	float distance = glm::length(direction);
	if (distance < 0.001f)
	{
		// 이미 waypoint에 도달함
		return glm::vec3(0.0f);
	}

	// 정규화된 방향 벡터 반환
	return glm::normalize(direction);
}

// ========================================
// Phase 1-1: UpdateMovement 구현
// ========================================
void AIController::UpdateMovement(float deltaTime)
{
	// 목표가 없거나 pathFinder가 없으면 IDLE
	if (!hasTarget || !pathFinder)
	{
		behaviorMode = BehaviorMode::IDLE;
		return;
	}

	// 경로가 유효하지 않으면 STUCK
	if (!currentPath.isValid || currentPath.waypoints.empty())
	{
		behaviorMode = BehaviorMode::STUCK;
		return;
	}

	// 경로가 완료되었는지 확인
	if (currentPath.IsComplete())
	{
		// 목표 도달 - 상태 업데이트
		hasTarget = false;
		behaviorMode = BehaviorMode::IDLE;
		currentPath.isValid = false;
		std::cout << "AIController: Target reached!" << std::endl;
		return;
	}

	// 다음 waypoint 조회
	glm::vec3 nextWaypoint = currentPath.GetNextWaypoint();
	if (nextWaypoint == glm::vec3(0.0f))
	{
		// Waypoint 없음 - 경로 끝
		currentPath.SetCurrentWaypointIndex(currentPath.GetCurrentWaypointIndex() + 1);
		return;
	}

	// 현재 위치에서 다음 waypoint까지의 거리
	float distanceToWaypoint = glm::distance(currentPosition, nextWaypoint);

	// Waypoint 도달 판정
	if (distanceToWaypoint < WAYPOINT_REACH_DISTANCE)
	{
		// 다음 waypoint로 진행
		int nextIndex = currentPath.GetCurrentWaypointIndex() + 1;
		currentPath.SetCurrentWaypointIndex(nextIndex);
		std::cout << "AIController: Waypoint reached. Moving to next waypoint." << std::endl;
		// ✅ 여기는 return 제거! 계속 이동해야 함
	}

	// ⭐ 이동 로직 (항상 실행)
	glm::vec3 moveDirection = GetNextMoveDirection();
	if (glm::length(moveDirection) > 0.001f)
	{
		// 이동 거리 계산 (속도 × 시간)
		float moveDistance = GameConstants::PROFESSOR_MOVE_SPEED * deltaTime;

		// 현재 위치 업데이트
		currentPosition += moveDirection * moveDistance;

		// 진행도 업데이트
		moveProgress += deltaTime;

		// ⭐ 디버그 출력 (주기적으로)
		static int moveCounter = 0;
		if (++moveCounter % 60 == 0) {
			std::cout << "AIController: Moving to ("
				<< currentPosition.x << ", "
				<< currentPosition.y << ", "
				<< currentPosition.z << ")" << std::endl;
		}
	}
}

// ========================================
// Phase 1-3: ChaseTarget 구현 (나중에 필요시 활용)
// ========================================
/*
void AIController::ChaseTarget(const glm::vec3& currentPos, const glm::vec3& targetPos, float maxChaseDistance, float deltaTime)
{
	if (!pathFinder)
		return;

	// 현재 위치 업데이트
	currentPosition = currentPos;

	// 현재 위치에서 대상까지의 거리 계산
	float distanceToTarget = glm::distance(currentPos, targetPos);

	// 거리가 최대 추격 거리를 초과했는지 확인
	if (distanceToTarget > maxChaseDistance)
	{
		// 최대 거리 초과 - 추격 포기
		ClearTarget();
		std::cout << "AIController: Target too far. Chase abandoned." << std::endl;
		return;
	}

	// 감지 범위 내 확인 (PROFESSOR_DETECTION_RANGE)
	if (distanceToTarget <= GameConstants::PROFESSOR_DETECTION_RANGE)
	{
		// 목표 설정
		if (!hasTarget)
		{
			SetTargetPosition(targetPos);
		}
		else
		{
			// 이미 추격 중이면 목표 위치 업데이트
			targetPosition = targetPos;
		}

		// 경로 계산 (주기적으로만 계산)
		lastPathUpdateTime += deltaTime;
		if (lastPathUpdateTime >= PATH_UPDATE_INTERVAL)
		{
			currentPath = pathFinder->FindPath(currentPos, targetPos);
			lastPathUpdateTime = 0.0f;

			if (!currentPath.isValid)
			{
				// 경로 찾기 실패 - STUCK 상태
				behaviorMode = BehaviorMode::STUCK;
				std::cout << "AIController: No path found. Stuck state." << std::endl;
			}
			else
			{
				// 경로 찾기 성공 - CHASING 모드 유지
				behaviorMode = BehaviorMode::CHASING;
				std::cout << "AIController: Path found. Chasing..." << std::endl;
			}
		}
	}
	else
	{
		// 감지 범위 밖 - IDLE
		ClearTarget();
		behaviorMode = BehaviorMode::IDLE;
	}
}
*/

// ========================================
// Phase 1-4: ValidateAndUpdatePath 구현
// ========================================
void AIController::ValidateAndUpdatePath(const glm::vec3& currentPos)
{
	if (!pathFinder || !hasTarget)
		return;

	// 현재 위치 업데이트
	currentPosition = currentPos;

	// 경로 유효성 확인
	if (!pathFinder->IsPathValid(currentPath))
	{
		std::cout << "AIController: Path is invalid. Recalculating..." << std::endl;

		// 새 경로 계산
		currentPath = pathFinder->FindPath(currentPos, targetPosition);

		if (!currentPath.isValid)
		{
			// 재계산 실패 - STUCK 상태
			behaviorMode = BehaviorMode::STUCK;
			std::cout << "AIController: Path recalculation failed. Stuck state." << std::endl;
		}
		else
		{
			// 재계산 성공 - CHASING 모드 유지
			behaviorMode = BehaviorMode::CHASING;
			std::cout << "AIController: Path recalculated successfully." << std::endl;
		}
	}
}

bool AIController::HasReachedTarget() const
{
	return currentPath.IsComplete();
}

// ⭐ 다음 waypoint 방향 미리보기
glm::vec3 AIController::GetUpcomingMoveDirection() const
{
	// 경로가 유효하지 않으면 현재 방향 반환
	if (!currentPath.isValid || currentPath.waypoints.empty())
	{
		return GetNextMoveDirection();
	}

	// 현재 waypoint 인덱스
	int currentIdx = currentPath.GetCurrentWaypointIndex();

	// 다음 waypoint가 있는지 확인
	if (currentIdx + 1 >= static_cast<int>(currentPath.waypoints.size()))
	{
		// 다음 waypoint가 없으면 현재 방향 반환
		return GetNextMoveDirection();
	}

	// ⭐ 다음 waypoint 방향 계산
	glm::vec3 nextWaypoint = currentPath.waypoints[currentIdx + 1];
	glm::vec3 direction = nextWaypoint - currentPosition;

	float distance = glm::length(direction);
	if (distance < 0.001f)
	{
		return GetNextMoveDirection();
	}

	return glm::normalize(direction);
}

// ⭐ 다음 waypoint까지의 남은 거리
float AIController::GetDistanceToNextWaypoint() const
{
	if (!currentPath.isValid || currentPath.waypoints.empty())
	{
		return 0.0f;
	}

	glm::vec3 nextWaypoint = currentPath.GetNextWaypoint();
	if (nextWaypoint == glm::vec3(0.0f))
	{
		return 0.0f;
	}

	return glm::distance(currentPosition, nextWaypoint);
}
