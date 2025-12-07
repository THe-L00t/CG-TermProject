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

	// ⭐ 디버그: Professor와 Player 위치 출력
	static int posDebugCounter = 0;
	if (posDebugCounter++ % 120 == 0) {
		std::cout << "\n===== PROFESSOR DEBUG =====" << std::endl;
		std::cout << "Professor position: (" << GetPosition().x << ", " << GetPosition().y << ", " << GetPosition().z << ")" << std::endl;
		if (playerRef != nullptr) {
			std::cout << "Player position: (" << playerRef->GetPosition().x << ", " << playerRef->GetPosition().y << ", " << playerRef->GetPosition().z << ")" << std::endl;
		}
		else {
			std::cout << "Player reference: NULL!" << std::endl;
		}
		if (aiController != nullptr) {
			std::cout << "AIController state: " << (int)aiController->GetBehaviorMode() << std::endl;
			std::cout << "AIController position: (" << aiController->GetCurrentPosition().x << ", " << aiController->GetCurrentPosition().y << ", " << aiController->GetCurrentPosition().z << ")" << std::endl;
		}
		else {
			std::cout << "AIController: NULL!" << std::endl;
		}
		std::cout << "============================\n" << std::endl;
	}

	if (playerRef != nullptr)
	{
		playerPosition = playerRef->GetPosition();
	}

	// AI 시스템 업데이트
	if (aiController != nullptr)
	{
		// 현재 위치를 AIController에 설정
		aiController->SetCurrentPosition(GetPosition());

		// ⭐ 디버그: 플레이어와의 거리 출력
		float distanceToPlayer = glm::distance(GetPosition(), playerPosition);
		static int debugCounter = 0;
		if (debugCounter++ % 60 == 0) {
			std::cout << "Professor: Distance to player = " << distanceToPlayer
				<< " / Detection range = " << detectionRange
				<< " / Behavior: " << (int)aiController->GetBehaviorMode() << std::endl;
		}

		// ⭐ 테스트용: 감지 범위를 무시하고 항상 도망
		if (true)  // ← 테스트용으로 항상 true
		{
			// IDLE 상태일 때만 목표 설정
			if (aiController->GetBehaviorMode() == AIController::BehaviorMode::IDLE)
			{
				std::cout << "Professor: Starting to flee (TEST MODE)..." << std::endl;
				std::cout << "  Current position: (" << GetPosition().x << ", " << GetPosition().y << ", " << GetPosition().z << ")" << std::endl;
				std::cout << "  Target position: (" << patrolTarget.x << ", " << patrolTarget.y << ", " << patrolTarget.z << ")" << std::endl;
				aiController->SetTargetPosition(patrolTarget);
			}
		}
		else
		{
			// 플레이어가 감지 범위 밖 - 목표 해제
			if (aiController->GetBehaviorMode() != AIController::BehaviorMode::IDLE)
			{
				std::cout << "Professor: Player out of range. Stopping." << std::endl;
				aiController->ClearTarget();
			}
		}

		// AI 업데이트 (이동 계산)
		aiController->UpdateMovement(deltaTime);

		// ⭐ AIController가 계산한 새 위치를 Professor에 적용
		glm::vec3 oldPosition = GetPosition();
		glm::vec3 newPosition = aiController->GetCurrentPosition();

		// ⭐ 위치가 실제로 변경되었는지 확인
		if (glm::length(newPosition - oldPosition) > 0.001f) {
			SetPosition(newPosition);
			static int moveDebugCounter = 0;
			if (moveDebugCounter++ % 30 == 0) {
				std::cout << "Professor: MOVED from (" << oldPosition.x << ", " << oldPosition.z
					<< ") to (" << newPosition.x << ", " << newPosition.z << ")" << std::endl;
			}
		}

		// 이동 방향 적용 (애니메이션용)
		glm::vec3 moveDirection = aiController->GetNextMoveDirection();
		if (glm::length(moveDirection) > 0.001f)
		{
			direction = moveDirection;
		}
	}
	else
	{
		static bool warnedOnce = false;
		if (!warnedOnce) {
			std::cout << "WARNING: Professor has no AIController!" << std::endl;
			warnedOnce = true;
		}
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