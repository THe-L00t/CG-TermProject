#include "CollisionManager.h"
#include "Object.h"
#include <iostream>

CollisionManager::CollisionManager()
{
}

CollisionManager::~CollisionManager()
{
}

void CollisionManager::RegisterDynamicObject(Object* object)
{
	if (object)
	{
		dynamicObjects.push_back(object);
	}
}

bool CollisionManager::CheckCollisionAt(const Object* movingObject, const glm::vec3& newPosition) const
{
	if (!movingObject) return false;

	// 현재 위치를 임시로 저장하고 새 위치로 설정
	glm::vec3 originalPos = movingObject->GetPosition();
	const_cast<Object*>(movingObject)->SetPosition(newPosition);

	// 새 위치에서의 바운딩 박스 계산 (각 Object의 커스텀 GetBoundingBox 사용)
	glm::vec3 movingMin, movingMax;
	movingObject->GetBoundingBox(movingMin, movingMax);

	// 원래 위치 복원
	const_cast<Object*>(movingObject)->SetPosition(originalPos);

	// 정적 오브젝트 그룹과 충돌 검사
	for (const auto* group : staticObjectVectors)
	{
		if (!group) continue; // null 체크

		for (const auto& obj : *group)
		{
			if (!obj) continue; // null 체크
			if (!obj->IsActive()) continue;

			try {
				glm::vec3 objMin, objMax;
				obj->GetBoundingBox(objMin, objMax);

				if (CheckAABB(movingMin, movingMax, objMin, objMax))
				{
					return true; // 충돌 발생
				}
			}
			catch (...) {
				// GetBoundingBox 호출 실패 시 무시하고 계속
				std::cerr << "CollisionManager: Exception in GetBoundingBox for static object" << std::endl;
				continue;
			}
		}
	}

	// 다른 동적 오브젝트와 충돌 검사 (자기 자신 제외)
	for (const auto* obj : dynamicObjects)
	{
		if (!obj) continue; // null 체크
		if (obj == movingObject || !obj->IsActive()) continue;

		try {
			glm::vec3 objMin, objMax;
			obj->GetBoundingBox(objMin, objMax);

			if (CheckAABB(movingMin, movingMax, objMin, objMax))
			{
				return true; // 충돌 발생
			}
		}
		catch (...) {
			// GetBoundingBox 호출 실패 시 무시하고 계속
			std::cerr << "CollisionManager: Exception in GetBoundingBox for dynamic object" << std::endl;
			continue;
		}
	}

	return false; // 충돌 없음
}

void CollisionManager::Update()
{
	// 현재는 충돌 검사만 수행 (충돌 반응은 각 오브젝트가 처리)
	// 필요시 여기서 충돌 이벤트를 발생시킬 수 있음
}

void CollisionManager::ClearAll()
{
	staticObjectVectors.clear();
	dynamicObjects.clear();
}

bool CollisionManager::CheckAABB(const glm::vec3& min1, const glm::vec3& max1,
                                  const glm::vec3& min2, const glm::vec3& max2) const
{
	bool collisionX = max1.x > min2.x && min1.x < max2.x;
	bool collisionY = max1.y > min2.y && min1.y < max2.y;
	bool collisionZ = max1.z > min2.z && min1.z < max2.z;

	return collisionX && collisionY && collisionZ;
}
