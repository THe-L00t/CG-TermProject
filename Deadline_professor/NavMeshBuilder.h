#pragma once

#include "TotalHeader.h"
#include "MapGenerator.h"

class NavMesh;
class MapGenerator;
class Wall;

// ========================================
// 네비게이션 메시 생성기
// ========================================
class NavMeshBuilder
{
public:
	NavMeshBuilder();
	~NavMeshBuilder();

	// 맵 데이터로부터 네비게이션 메시 생성
	std::unique_ptr<NavMesh> Build(const MapGenerator* mapGenerator);

	// ⭐ 실제 3D 벽 객체로부터 네비게이션 메시 생성 (새로운 메서드)
	void BuildFromWalls(const std::vector<std::unique_ptr<Wall>>* walls,
		const glm::vec3& playerStartPos,
		float tileSize);

	// NavMesh 포인터 반환 (BuildFromWalls 후 접근)
	NavMesh* GetNavMesh() const { return builtNavMesh.get(); }

	// 디버그 정보 출력
	void PrintNavMeshStats(const NavMesh* navMesh) const;

private:
	// 빌드된 NavMesh 저장
	std::unique_ptr<NavMesh> builtNavMesh;

	// 유틸리티 함수
	bool IsTileWalkable(TileType tileType) const;

	// ⭐ 점이 벽 내부에 있는지 확인
	bool IsPointInsideAnyWall(const glm::vec3& point,
		const std::vector<std::unique_ptr<Wall>>* walls,
		float collisionRadius) const;

	// ⭐ 2개 점 사이에 벽이 있는지 확인 (직선 교차)
	bool IsLineClearOfWalls(const glm::vec3& from,
		const glm::vec3& to,
		const std::vector<std::unique_ptr<Wall>>* walls) const;
};
