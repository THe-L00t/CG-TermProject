#pragma once

#include "TotalHeader.h"
#include "MapGenerator.h"

class NavMesh;
class MapGenerator;

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

	// 디버그 정보 출력
	void PrintNavMeshStats(const NavMesh* navMesh) const;

private:
	// 유틸리티 함수
	bool IsTileWalkable(TileType tileType) const;
};
