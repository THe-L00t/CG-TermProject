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
	}
}

void AIController::ClearTarget()
{
	hasTarget = false;
	behaviorMode = BehaviorMode::IDLE;
	currentPath.isValid = false;
}

void AIController::UpdateMovement(float deltaTime)
{
	if (!hasTarget || !pathFinder)
	{
		behaviorMode = BehaviorMode::IDLE;
		return;
	}

	// 경로가 유효하지 않으면 새로 계산 (나중에 구현)
	// 경로 진행도 업데이트
	moveProgress += deltaTime;
}

glm::vec3 AIController::GetNextMoveDirection() const
{
	if (!currentPath.isValid || currentPath.waypoints.empty())
	{
		return glm::vec3(0.0f);
	}

	// 다음 목표점 방향 계산
	glm::vec3 nextWaypoint = currentPath.GetNextWaypoint();
	if (nextWaypoint == glm::vec3(0.0f))
	{
		return glm::vec3(0.0f);
	}

	// 방향 벡터 정규화 (정확한 구현은 나중에)
	return glm::normalize(nextWaypoint);
}

bool AIController::HasReachedTarget() const
{
	return currentPath.IsComplete();
}

void AIController::ChaseTarget(const glm::vec3& targetPos, float maxChaseDistance, float deltaTime)
{
	if (!pathFinder)
		return;

	// 이 함수는 추후 구현
	// 플레이어를 일정 거리까지 추적하되, maxChaseDistance를 초과하면 추적 중단
}

void AIController::ValidateAndUpdatePath(const glm::vec3& currentPos)
{
	if (!pathFinder)
		return;

	// 현재 경로가 유효한지 확인
	if (!pathFinder->IsPathValid(currentPath))
	{
		// 새 경로 계산
		if (hasTarget)
		{
			currentPath = pathFinder->FindPath(currentPos, targetPosition,
				GameConstants::PROFESSOR_DETECTION_RANGE);
		}
	}
}
