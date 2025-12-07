#include "Professor.h"
#include "GameConstants.h"
#include "PathFinder.h"
#include "NavMesh.h"

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

		// ⭐ 플레이어 감지 및 동적 목표 업데이트
		float distanceToPlayer = glm::distance(GetPosition(), playerPosition);

		// ⭐⭐⭐ 목표 업데이트 타이머 (2초마다 한 번만 업데이트)
		static float targetUpdateTimer = 0.0f;
		const float TARGET_UPDATE_INTERVAL = 2.0f;  // 2초

		targetUpdateTimer += deltaTime;

		if (true)  // 테스트 모드
		{
			AIController::BehaviorMode currentMode = aiController->GetBehaviorMode();

			// ✅ IDLE 상태면 즉시 목표 설정
			if (currentMode == AIController::BehaviorMode::IDLE)
			{
				std::cout << "Professor: IDLE state - Setting initial target!" << std::endl;
				glm::vec3 dynamicEscapeTarget = CalculateEscapeTarget(GetPosition(), playerPosition);
				aiController->SetTargetPosition(dynamicEscapeTarget);
				targetUpdateTimer = 0.0f;  // 타이머 리셋
			}
			// ✅ STUCK 상태면 즉시 재시도
			else if (currentMode == AIController::BehaviorMode::STUCK)
			{
				std::cout << "Professor: STUCK - Retrying!" << std::endl;
				glm::vec3 dynamicEscapeTarget = CalculateEscapeTarget(GetPosition(), playerPosition);
				aiController->SetTargetPosition(dynamicEscapeTarget);
				targetUpdateTimer = 0.0f;  // 타이머 리셋
			}
			// ✅ CHASING 상태면 타이머 & 거리 조건 모두 확인
			else if (currentMode == AIController::BehaviorMode::CHASING)
			{
				float distanceToWaypoint = aiController->GetDistanceToNextWaypoint();

				// ⭐ 타이머가 지났고, 목표에 가까워졌을 때만 업데이트
				if (targetUpdateTimer >= TARGET_UPDATE_INTERVAL && distanceToWaypoint < 8.0f)
				{
					std::cout << "Professor: Updating target (timer: " << targetUpdateTimer
						<< "s, distance: " << distanceToWaypoint << "m)" << std::endl;
					glm::vec3 dynamicEscapeTarget = CalculateEscapeTarget(GetPosition(), playerPosition);
					aiController->SetTargetPosition(dynamicEscapeTarget);
					targetUpdateTimer = 0.0f;  // 타이머 리셋
				}
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

		// ⭐⭐⭐ 예측 회전 구현 (동일)
		glm::vec3 currentMoveDirection = aiController->GetNextMoveDirection();
		glm::vec3 upcomingMoveDirection = aiController->GetUpcomingMoveDirection();
		float distanceToWaypoint = aiController->GetDistanceToNextWaypoint();

		const float LOOK_AHEAD_TIME = 0.35f;
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

			float targetAngleY = atan2f(-targetDirection.x, -targetDirection.z);
			float targetAngleDegrees = glm::degrees(targetAngleY);

			glm::vec3 currentRotation = GetRotation();
			float currentAngleDegrees = currentRotation.y;

			float angleDiff = targetAngleDegrees - currentAngleDegrees;
			while (angleDiff > 180.0f) angleDiff -= 360.0f;
			while (angleDiff < -180.0f) angleDiff += 360.0f;

			const float rotationSpeed = 3.33f;
			float lerpFactor = glm::min(deltaTime * rotationSpeed, 1.0f);

			float newAngleDegrees = currentAngleDegrees + (angleDiff * lerpFactor);

			SetRotation(glm::vec3(0.0f, newAngleDegrees, 0.0f));
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

// ⭐ 플레이어 반대 방향으로 탈출 목표 계산 (NavMesh 검증 포함)
glm::vec3 Professor::CalculateEscapeTarget(const glm::vec3& npcPos, const glm::vec3& playerPos)
{
	// 플레이어 → NPC 방향 벡터
	glm::vec3 fleeDirection = npcPos - playerPos;

	// XZ 평면만 사용 (Y축 무시)
	fleeDirection.y = 0.0f;

	// 정규화
	if (glm::length(fleeDirection) > 0.001f)
	{
		fleeDirection = glm::normalize(fleeDirection);
	}
	else
	{
		// 플레이어와 같은 위치면 랜덤 방향
		fleeDirection = glm::vec3(1.0f, 0.0f, 0.0f);
	}

	// ⭐ 도망갈 거리 설정
	const float FLEE_DISTANCE = 20.0f * GameConstants::TILE_SIZE; // 80m

	// 이상적인 탈출 목표
	glm::vec3 idealTarget = npcPos + (fleeDirection * FLEE_DISTANCE);
	idealTarget.y = 0.0f;

	// 맵 경계 내로 제한
	float halfMapSize = (GameConstants::MAP_GRID_WIDTH * GameConstants::TILE_SIZE) * 0.5f;
	idealTarget.x = glm::clamp(idealTarget.x, -halfMapSize + 4.0f, halfMapSize - 4.0f);
	idealTarget.z = glm::clamp(idealTarget.z, -halfMapSize + 4.0f, halfMapSize - 4.0f);

	// ⭐⭐⭐ NavMesh에서 가장 가까운 이동 가능한 노드 찾기
	if (pathFinder && pathFinder->GetNavMesh())
	{
		NavMesh* navMesh = pathFinder->GetNavMesh();
		NavNode* targetNode = navMesh->GetNodeFromWorldPos(idealTarget);

		// 목표 노드가 이동 가능하면 그대로 사용
		if (targetNode && targetNode->IsWalkable())
		{
			return targetNode->GetWorldPosition();
		}

		// ⭐ 목표 노드가 벽이면 주변에서 이동 가능한 노드 찾기
		std::cout << "Professor: Ideal target is blocked. Searching nearby walkable node..." << std::endl;

		// 반경을 점점 넓혀가며 검색 (1타일 → 10타일)
		for (int radius = 1; radius <= 10; ++radius)
		{
			for (int dz = -radius; dz <= radius; ++dz)
			{
				for (int dx = -radius; dx <= radius; ++dx)
				{
					// 현재 반경의 테두리만 검사 (이미 검사한 내부는 스킵)
					if (std::abs(dx) != radius && std::abs(dz) != radius)
						continue;

					// 테스트 위치 계산
					glm::vec3 testPos = idealTarget + glm::vec3(
						dx * GameConstants::TILE_SIZE,
						0.0f,
						dz * GameConstants::TILE_SIZE
					);

					// NavMesh에서 노드 확인
					NavNode* testNode = navMesh->GetNodeFromWorldPos(testPos);
					if (testNode && testNode->IsWalkable())
					{
						std::cout << "Professor: Found walkable node at radius " << radius
							<< " tiles, position (" << testPos.x << ", " << testPos.z << ")" << std::endl;
						return testNode->GetWorldPosition();
					}
				}
			}
		}

		// ⭐ 최후의 수단: 현재 위치에서 같은 방향으로 더 가까운 거리
		std::cout << "Professor: No walkable node found. Using shorter distance..." << std::endl;
		glm::vec3 fallbackTarget = npcPos + (fleeDirection * (FLEE_DISTANCE * 0.5f));
		fallbackTarget.x = glm::clamp(fallbackTarget.x, -halfMapSize + 4.0f, halfMapSize - 4.0f);
		fallbackTarget.z = glm::clamp(fallbackTarget.z, -halfMapSize + 4.0f, halfMapSize - 4.0f);
		fallbackTarget.y = 0.0f;

		targetNode = navMesh->GetNodeFromWorldPos(fallbackTarget);
		if (targetNode && targetNode->IsWalkable())
		{
			return targetNode->GetWorldPosition();
		}
	}

	// ⭐ NavMesh가 없거나 모두 실패하면 이상적인 목표 반환
	return idealTarget;
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