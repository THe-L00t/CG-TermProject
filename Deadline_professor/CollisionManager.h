#pragma once
#include "TotalHeader.h"
#include <vector>
#include <memory>

class Object;
class Wall;

// 충돌 관리 클래스
// 단일 책임: 게임 내 모든 오브젝트 간 충돌 검사 및 관리
class CollisionManager
{
public:
	CollisionManager();
	~CollisionManager();

	// 충돌 체크할 오브젝트 그룹 등록 (템플릿으로 다양한 타입 지원)
	template<typename T>
	void RegisterStaticObjects(const std::vector<std::unique_ptr<T>>* objects)
	{
		if (objects)
		{
			// Object 포인터 벡터로 변환하여 저장
			staticObjectVectors.push_back(reinterpret_cast<const std::vector<std::unique_ptr<Object>>*>(objects));
		}
	}

	void RegisterDynamicObject(Object* object);

	// 특정 위치에서 충돌 검사 (주로 이동 전 검사용)
	bool CheckCollisionAt(const Object* movingObject, const glm::vec3& newPosition) const;

	// 등록된 모든 오브젝트 간 충돌 검사
	void Update();

	// 등록 해제
	void ClearAll();

private:
	// AABB 충돌 검사
	bool CheckAABB(const glm::vec3& min1, const glm::vec3& max1,
	               const glm::vec3& min2, const glm::vec3& max2) const;

	// 정적 오브젝트 그룹 (벽, 장애물 등)
	std::vector<const std::vector<std::unique_ptr<Object>>*> staticObjectVectors;

	// 동적 오브젝트 (플레이어, 교수님 등)
	std::vector<Object*> dynamicObjects;
};
