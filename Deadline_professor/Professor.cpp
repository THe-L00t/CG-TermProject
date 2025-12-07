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

		// ⭐ 플레이어 감지 및 목표 설정
		float distanceToPlayer = glm::distance(GetPosition(), playerPosition);
		static int debugCounter = 0;
		if (debugCounter++ % 60 == 0) {
			std::cout << "Professor: Distance to player = " << distanceToPlayer
				<< " / Detection range = " << detectionRange
				<< " / Behavior: " << (int)aiController->GetBehaviorMode() << std::endl;
		}

		// ⭐ 테스트용: 항상 도망
		// if (distanceToPlayer <= detectionRange)
		if (true)
		{
			if (aiController->GetBehaviorMode() == AIController::BehaviorMode::IDLE)
			{
				std::cout << "Professor: Starting to flee (TEST MODE)..." << std::endl;
				aiController->SetTargetPosition(patrolTarget);
			}
		}

		// AI 업데이트 (이동 계산)
		aiController->UpdateMovement(deltaTime);

		// ⭐ 위치 업데이트
		glm::vec3 oldPosition = GetPosition();
		glm::vec3 newPosition = aiController->GetCurrentPosition();

		if (glm::length(newPosition - oldPosition) > 0.001f) {
			SetPosition(newPosition);
		}

		// ⭐⭐⭐ 예측 회전 구현!
		glm::vec3 currentMoveDirection = aiController->GetNextMoveDirection();
		glm::vec3 upcomingMoveDirection = aiController->GetUpcomingMoveDirection();
		float distanceToWaypoint = aiController->GetDistanceToNextWaypoint();

		// ⭐ 회전할 방향 결정
		const float LOOK_AHEAD_TIME = 0.35f; // 0.35초 전부터 회전 시작
		const float LOOK_AHEAD_DISTANCE = GameConstants::PROFESSOR_MOVE_SPEED * LOOK_AHEAD_TIME;

		glm::vec3 targetDirection;
		if (distanceToWaypoint > 0.001f && distanceToWaypoint < LOOK_AHEAD_DISTANCE)
		{
			targetDirection = upcomingMoveDirection;
		}
		else
		{
			targetDirection = currentMoveDirection;
		}

		if (glm::length(targetDirection) > 0.001f)
		{
			direction = targetDirection;

			// 목표 회전 각도 계산 (180도 반전 적용)
			float targetAngleY = atan2f(-targetDirection.x, -targetDirection.z);
			float targetAngleDegrees = glm::degrees(targetAngleY);

			// 현재 회전 각도
			glm::vec3 currentRotation = GetRotation();
			float currentAngleDegrees = currentRotation.y;

			// 각도 차이 계산 (최단 경로)
			float angleDiff = targetAngleDegrees - currentAngleDegrees;
			while (angleDiff > 180.0f) angleDiff -= 360.0f;
			while (angleDiff < -180.0f) angleDiff += 360.0f;

			// 부드러운 회전 (0.3초 동안 완료)
			const float rotationSpeed = 3.33f; // 1.0 / 0.3
			float lerpFactor = glm::min(deltaTime * rotationSpeed, 1.0f);

			float newAngleDegrees = currentAngleDegrees + (angleDiff * lerpFactor);

			SetRotation(glm::vec3(0.0f, newAngleDegrees, 0.0f));

			// 디버그 출력
			static int rotDebugCounter = 0;
			if (rotDebugCounter++ % 60 == 0) {
				std::cout << "Professor: Distance to waypoint: " << distanceToWaypoint
					<< "m, Using " << (distanceToWaypoint < LOOK_AHEAD_DISTANCE ? "UPCOMING" : "CURRENT")
					<< " direction" << std::endl;
			}
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