#include "NavMesh.h"
#include "GameConstants.h"

// ========================================
// NavNode 구현
// ========================================
NavNode::NavNode(int gridX, int gridZ, bool walkable)
	: gridX(gridX), gridZ(gridZ), walkable(walkable)
{
}

glm::vec3 NavNode::GetWorldPosition() const
{
	// 맵 중심을 원점으로 하는 월드 좌표 변환
	float halfMapWidth = (GameConstants::MAP_GRID_WIDTH * GameConstants::TILE_SIZE) * 0.5f;
	float halfMapDepth = (GameConstants::MAP_GRID_DEPTH * GameConstants::TILE_SIZE) * 0.5f;

	float worldX = (gridX * GameConstants::TILE_SIZE) - halfMapWidth + (GameConstants::TILE_SIZE * 0.5f);
	float worldZ = (gridZ * GameConstants::TILE_SIZE) - halfMapDepth + (GameConstants::TILE_SIZE * 0.5f);

	return glm::vec3(worldX, 0.0f, worldZ);
}

// ========================================
// NavMesh 구현
// ========================================
NavMesh::NavMesh(int gridWidth, int gridDepth)
	: gridWidth(gridWidth), gridDepth(gridDepth)
{
	// gridWidth * gridDepth 개의 노드 미리 할당
	nodes.resize(gridWidth * gridDepth);
	for (int z = 0; z < gridDepth; ++z)
	{
		for (int x = 0; x < gridWidth; ++x)
		{
			int index = CoordToIndex(x, z);
			nodes[index] = std::make_unique<NavNode>(x, z, false); // 초기값: 이동 불가
		}
	}
}

NavMesh::~NavMesh()
{
}

void NavMesh::Initialize(const std::vector<std::vector<TileType>>& mapData)
{
	std::cout << "\n===== NAVMESH INITIALIZATION START =====" << std::endl;

	// 1단계: 타일 데이터 기반 이동 가능 영역 설정
	int walkableTileCount = 0;
	for (int z = 0; z < gridDepth; ++z)
	{
		for (int x = 0; x < gridWidth; ++x)
		{
			TileType tileType = mapData[z][x];

			// WALL이 아닌 모든 타일은 기본적으로 이동 가능
			bool canWalk = (tileType != TileType::WALL);

			if (canWalk)
			{
				// 2단계: NPC 충돌 영역 검증 (현재는 단일 타일만 확인)
				// 향후: 인접 타일들도 함께 검증할 수 있음
				if (IsCollisionFree(x, z))
				{
					NavNode* node = GetNode(x, z);
					if (node)
					{
						node->SetWalkable(true);
						walkableTileCount++;
					}
				}
			}
		}
	}

	// 3단계: 인접 노드 관계 설정 (그래프 구성)
	ConnectNeighbors();

	std::cout << "v Walkable tiles: " << walkableTileCount << " / " << (gridWidth * gridDepth) << std::endl;
	std::cout << "v NavMesh graph initialized" << std::endl;
	std::cout << "========================================\n" << std::endl;
}

NavNode* NavMesh::GetNode(int gridX, int gridZ) const
{
	// 범위 검증
	if (gridX < 0 || gridX >= gridWidth || gridZ < 0 || gridZ >= gridDepth)
	{
		return nullptr;
	}

	int index = CoordToIndex(gridX, gridZ);
	return GetNodeInternal(index);
}

NavNode* NavMesh::GetNodeFromWorldPos(const glm::vec3& worldPos) const
{
	// 월드 좌표 → 그리드 좌표 변환
	float halfMapWidth = (gridWidth * GameConstants::TILE_SIZE) * 0.5f;
	float halfMapDepth = (gridDepth * GameConstants::TILE_SIZE) * 0.5f;

	int gridX = static_cast<int>((worldPos.x + halfMapWidth) / GameConstants::TILE_SIZE);
	int gridZ = static_cast<int>((worldPos.z + halfMapDepth) / GameConstants::TILE_SIZE);

	// 경계값 보정 (음수 또는 범위 초과 방지) - 수동으로 범위 제한
	gridX = std::max(0, std::min(gridX, gridWidth - 1));
	gridZ = std::max(0, std::min(gridZ, gridDepth - 1));

	return GetNode(gridX, gridZ);
}

float NavMesh::GetTileSize() const
{
	return GameConstants::TILE_SIZE;
}

bool NavMesh::IsWalkableArea(int gridX, int gridZ) const
{
	NavNode* node = GetNode(gridX, gridZ);
	if (!node) return false;
	return node->IsWalkable();
}

bool NavMesh::CanMoveBetween(int fromX, int fromZ, int toX, int toZ) const
{
	// 현재 구현: 두 지점 모두 이동 가능하면 이동 가능
	// 향후: 경로상의 모든 타일 검증 추가 가능
	return IsWalkableArea(fromX, fromZ) && IsWalkableArea(toX, toZ);
}

NavNode* NavMesh::GetNodeInternal(int index) const
{
	if (index < 0 || index >= static_cast<int>(nodes.size()))
	{
		return nullptr;
	}
	return nodes[index].get();
}

int NavMesh::CoordToIndex(int gridX, int gridZ) const
{
	return gridZ * gridWidth + gridX;
}

void NavMesh::CoordFromIndex(int index, int& gridX, int& gridZ) const
{
	gridZ = index / gridWidth;
	gridX = index % gridWidth;
}

void NavMesh::ConnectNeighbors()
{
	std::cout << "Connecting neighbors in NavMesh..." << std::endl;

	// 모든 노드의 이웃 초기화
	for (auto& node : nodes)
	{
		if (node)
		{
			node->ClearNeighbors();
		}
	}

	// 4방향 인접 노드 연결 (상하좌우)
	const int dx[] = { 0, 1, 0, -1 };  // 상, 우, 하, 좌
	const int dz[] = { -1, 0, 1, 0 };

	for (int z = 0; z < gridDepth; ++z)
	{
		for (int x = 0; x < gridWidth; ++x)
		{
			NavNode* currentNode = GetNode(x, z);
			if (!currentNode || !currentNode->IsWalkable())
				continue;

			// 4방향 이웃 연결
			for (int dir = 0; dir < 4; ++dir)
			{
				int nx = x + dx[dir];
				int nz = z + dz[dir];

				NavNode* neighbor = GetNode(nx, nz);
				if (neighbor && neighbor->IsWalkable())
				{
					currentNode->AddNeighbor(neighbor);
				}
			}
		}
	}

	std::cout << "v All neighbors connected" << std::endl;
}

bool NavMesh::IsCollisionFree(int centerX, int centerZ) const
{
	// NPC의 충돌 영역: PLAYER_HEIGHT × PLAYER_WIDTH
	// 임시로 단순 구현: 현재 타일이 이동 가능하면 충돌 없음
	// 
	// 향후 개선: 실제 충돌 체크
	// - centerX, centerZ를 기준으로 주변 영역 확인
	// - NPC의 폭(PLAYER_WIDTH)만큼 인접 타일 검증
	// - 현재는 간단하게 단일 타일만 확인

	// 플레이어 검증: 주변 타일도 안전한지 확인할 수 있음
	// 예: 반경 1타일 이내에 WALL이 없는지 확인
	// (지금은 구현하지 않음 - 필요시 추가)

	return true; // 임시: 모든 non-WALL 타일은 이동 가능
}
