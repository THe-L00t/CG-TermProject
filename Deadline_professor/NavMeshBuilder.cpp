#include "NavMeshBuilder.h"
#include "NavMesh.h"
#include "MapGenerator.h"
#include "GameConstants.h"

NavMeshBuilder::NavMeshBuilder()
{
}

NavMeshBuilder::~NavMeshBuilder()
{
}

std::unique_ptr<NavMesh> NavMeshBuilder::Build(const MapGenerator* mapGenerator)
{
	if (!mapGenerator)
	{
		std::cerr << "ERROR: NavMeshBuilder - mapGenerator is null" << std::endl;
		return nullptr;
	}

	std::cout << "\n===== NAVMESH BUILDING START =====" << std::endl;

	// 1단계: NavMesh 객체 생성
	auto navMesh = std::make_unique<NavMesh>(
		GameConstants::MAP_GRID_WIDTH,
		GameConstants::MAP_GRID_DEPTH
	);

	// 2단계: MapGenerator의 맵 데이터 추출
	const auto& mapData = mapGenerator->GetMap();

	// 3단계: NavMesh 초기화 (내부적으로 그래프 구성)
	navMesh->Initialize(mapData);

	// 4단계: 디버그 정보 출력
	PrintNavMeshStats(navMesh.get());

	std::cout << "===== NAVMESH BUILDING COMPLETE =====" << std::endl;

	return navMesh;
}

void NavMeshBuilder::PrintNavMeshStats(const NavMesh* navMesh) const
{
	if (!navMesh)
	{
		std::cerr << "ERROR: NavMesh is null" << std::endl;
		return;
	}

	std::cout << "\n===== NAVMESH STATISTICS =====" << std::endl;
	std::cout << "Grid size: " << navMesh->GetGridWidth() << " x " << navMesh->GetGridDepth() << std::endl;
	std::cout << "Tile size: " << navMesh->GetTileSize() << " units" << std::endl;

	// 이동 가능한 노드 개수 카운트
	int walkableCount = 0;
	const auto& allNodes = navMesh->GetAllNodes();
	for (const auto& node : allNodes)
	{
		if (node && node->IsWalkable())
		{
			walkableCount++;
		}
	}

	std::cout << "Walkable nodes: " << walkableCount << " / " << allNodes.size() << std::endl;
	std::cout << "===============================\n" << std::endl;
}

bool NavMeshBuilder::IsTileWalkable(TileType tileType) const
{
	// WALL이 아닌 모든 타일은 이동 가능
	return tileType != TileType::WALL;
}
