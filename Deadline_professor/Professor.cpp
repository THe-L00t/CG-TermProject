#include "Professor.h"
#include "GameConstants.h"
#include "PathFinder.h"

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

	// AI 시스템 업데이트
	if (aiController != nullptr)
	{
		// 현재 위치를 AIController에 설정
		aiController->SetCurrentPosition(GetPosition());

		// 플레이어 감지 범위 내에서만 도망
		float distanceToPlayer = glm::distance(GetPosition(), playerPosition);
		if (distanceToPlayer <= detectionRange)
		{
			// 플레이어가 감지 범위 내 - 목표 설정
			aiController->SetTargetPosition(patrolTarget);
		}
		else
		{
			// 플레이어가 감지 범위 밖 - 목표 해제
			aiController->ClearTarget();
		}

		// AI 업데이트
		aiController->UpdateMovement(deltaTime);

		// 이동 방향 적용
		glm::vec3 moveDirection = aiController->GetNextMoveDirection();
		if (glm::length(moveDirection) > 0.001f)
		{
			// 방향 설정 (회전 애니메이션에 필요)
			direction = moveDirection;

			// 위치 업데이트
			glm::vec3 newPosition = GetPosition() + moveDirection * moveSpeed * deltaTime;
			SetPosition(newPosition);
		}
	}
	else
	{
		// AIController가 없으면 기존 로직 사용
		FleeFromPlayer(deltaTime);
	}
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

// ========================================
// AI 시스템 관련 메서드
// ========================================

void Professor::SetAIController(AIController* controller)
{
	aiController = controller;
}

AIController* Professor::GetAIController() const
{
	return aiController;
}

void Professor::SetPatrolTarget(const glm::vec3& targetPos)
{
	patrolTarget = targetPos;
}

void Professor::FleeFromPlayer(float deltaTime)
{
	// AIController가 없을 때 사용되는 기본 동작
	// 현재는 구현되지 않음 (AIController 사용 권장)
	// Maybe Just stand (IDLE 상태)
}

void Professor::SetPathFinder(PathFinder* pf)
{
	pathFinder = pf;
}

PathFinder* Professor::GetPathFinder() const
{
	return pathFinder;
}

// 사용 예시
/*
// Floor1Scene에서
Professor* professor = new Professor("RunLee", "RunLee");

// PathFinder 생성 (NavMesh 기반)
PathFinder pathFinder(navMesh);

// AIController 생성
AIController* aiController = new AIController(&pathFinder);

// Professor에 AIController 설정
professor->SetAIController(aiController);

// 도망칠 목표 지점 설정 (맵의 특정 위치)
professor->SetPatrolTarget(glm::vec3(10.0f, 0.0f, 10.0f));
*/