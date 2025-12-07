#include "NavMeshBuilder.h"
#include "NavMesh.h"
#include "MapGenerator.h"
#include "Wall.h"
#include "GameConstants.h"

NavMeshBuilder::NavMeshBuilder()
	: builtNavMesh(nullptr)
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

	std::cout << "\n===== NAVMESH BUILDING START (MapGenerator) =====" << std::endl;

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

// ⭐⭐⭐ 새로운 메서드: 실제 3D 벽 객체 기반 NavMesh 생성
void NavMeshBuilder::BuildFromWalls(const std::vector<std::unique_ptr<Wall>>* walls,
	const glm::vec3& playerStartPos,
	float tileSize)
{
	if (!walls || walls->empty()) {
		std::cerr << "ERROR: NavMeshBuilder::BuildFromWalls - No walls provided!" << std::endl;
		return;
	}

	std::cout << "\n===== NAVMESH BUILDING START (FROM 3D WALLS) =====" << std::endl;
	std::cout << "Total walls: " << walls->size() << std::endl;

	// 1단계: NavMesh 생성
	builtNavMesh = std::make_unique<NavMesh>(
		GameConstants::MAP_GRID_WIDTH,
		GameConstants::MAP_GRID_DEPTH
	);

	// 2단계: 타일별로 이동 가능 여부 판단
	float halfMapSize = (GameConstants::MAP_GRID_WIDTH * tileSize) * 0.5f;
	float collisionRadius = tileSize * 0.3f; // NPC 충돌 반지름

	int walkableNodeCount = 0;

	std::cout << "Analyzing " << (GameConstants::MAP_GRID_WIDTH * GameConstants::MAP_GRID_DEPTH)
		<< " tiles..." << std::endl;

	for (int z = 0; z < GameConstants::MAP_GRID_DEPTH; ++z) {
		for (int x = 0; x < GameConstants::MAP_GRID_WIDTH; ++x) {
			// 타일 중심 월드 좌표 계산
			float worldX = (x * tileSize) - halfMapSize + (tileSize * 0.5f);
			float worldZ = (z * tileSize) - halfMapSize + (tileSize * 0.5f);
			glm::vec3 tileCenter(worldX, 0.5f, worldZ);

			// 이 타일이 벽과 겹치는지 확인
			bool isWalkable = !IsPointInsideAnyWall(tileCenter, walls, collisionRadius);

			if (isWalkable) {
				walkableNodeCount++;
			}
			else {
				// 벽이 있는 위치의 노드를 비이동으로 표시
				NavNode* node = builtNavMesh->GetNode(x, z);
				if (node) {
					node->SetWalkable(false);
				}
			}
		}
	}

	std::cout << "Walkable nodes: " << walkableNodeCount << std::endl;

	// 3단계: 인접 노드 연결 (내부적으로 처리됨)
	std::cout << "Connecting neighbors..." << std::endl;

	// 4단계: 디버그 정보 출력
	PrintNavMeshStats(builtNavMesh.get());

	std::cout << "===== NAVMESH BUILDING COMPLETE (FROM 3D WALLS) =====" << std::endl;
}

// ⭐ 점이 벽 내부에 있는지 확인
bool NavMeshBuilder::IsPointInsideAnyWall(const glm::vec3& point,
	const std::vector<std::unique_ptr<Wall>>* walls,
	float collisionRadius) const
{
	for (const auto& wall : *walls) {
		if (!wall) continue;

		// 벽의 바운딩박스 가져오기
		glm::vec3 wallMin, wallMax;
		wall->GetBoundingBox(wallMin, wallMax);

		// 충돌 반지름만큼 확장된 바운딩박스
		glm::vec3 expandedMin = wallMin - glm::vec3(collisionRadius);
		glm::vec3 expandedMax = wallMax + glm::vec3(collisionRadius);

		// AABB 충돌 검사
		if (point.x >= expandedMin.x && point.x <= expandedMax.x &&
			point.y >= expandedMin.y && point.y <= expandedMax.y &&
			point.z >= expandedMin.z && point.z <= expandedMax.z) {
			return true; // 벽 내부
		}
	}
	return false; // 벽 외부
}

// ⭐ 직선이 벽을 통과하는지 확인 (추후 최적화용)
bool NavMeshBuilder::IsLineClearOfWalls(const glm::vec3& from,
	const glm::vec3& to,
	const std::vector<std::unique_ptr<Wall>>* walls) const
{
	// 간단한 구현: 중점만 확인
	glm::vec3 midpoint = (from + to) * 0.5f;
	return !IsPointInsideAnyWall(midpoint, walls, GameConstants::TILE_SIZE * 0.2f);
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
	std::cout << "Coverage: " << (100.0f * walkableCount / allNodes.size()) << "%" << std::endl;
	std::cout << "===============================\n" << std::endl;
}

bool NavMeshBuilder::IsTileWalkable(TileType tileType) const
{
	// WALL이 아닌 모든 타일은 이동 가능
	return tileType != TileType::WALL;
}
