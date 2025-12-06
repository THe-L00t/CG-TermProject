#include "MapGenerator.h"
#include <iostream>
#include <algorithm>

MapGenerator::MapGenerator(int width, int depth)
	: mapWidth(width), mapHeight(depth)
{
	// 1×1 타일 기반 미로 (슈퍼타일 = 실제 타일)
	// 예: 30×30 맵 → 28×28 사용 가능 (가장자리 제외)
	int usableWidth = mapWidth - 2;   // 가장자리 제외
	int usableHeight = mapHeight - 2;
	superGridWidth = usableWidth;
	superGridHeight = usableHeight;

	// 맵 초기화
	map.resize(mapHeight, std::vector<TileType>(mapWidth, TileType::WALL));
	superGrid.resize(superGridHeight, std::vector<SuperTile>(superGridWidth));

	// 슈퍼타일 그리드 초기화 (1:1 매핑)
	for (int z = 0; z < superGridHeight; ++z) {
		for (int x = 0; x < superGridWidth; ++x) {
			int worldX = 1 + x;  // 가장자리 1칸 제외
			int worldZ = 1 + z;
			superGrid[z][x] = SuperTile(x, z, worldX, worldZ);
		}
	}

	// 랜덤 생성기 초기화
	std::random_device rd;
	rng.seed(rd());

	std::cout << "MapGenerator: Initialized (" << mapWidth << "x" << mapHeight << ")" << std::endl;
	std::cout << "  Grid: " << superGridWidth << "x" << superGridHeight << " (1x1 tiles)" << std::endl;
}

MapGenerator::~MapGenerator()
{
}

void MapGenerator::Generate()
{
	std::cout << "MapGenerator: Starting 1-tile maze generation..." << std::endl;
	std::cout << "Map size: " << mapWidth << " x " << mapHeight << " tiles" << std::endl;

	InitializeMap();
	PlaceBorderWalls();
	PlaceStairs();
	GenerateMaze();
	// ExtractSinglePath(); // 1칸 미로에서는 불필요
	// FillRestWithWalls();  // 1칸 미로에서는 불필요

	std::cout << "MapGenerator: 1-tile maze generation complete!" << std::endl;
}

void MapGenerator::InitializeMap()
{
	// 모든 타일을 벽으로 초기화
	for (int z = 0; z < mapHeight; ++z) {
		for (int x = 0; x < mapWidth; ++x) {
			map[z][x] = TileType::WALL;
		}
	}

	// 슈퍼타일 방문 상태 초기화
	for (int z = 0; z < superGridHeight; ++z) {
		for (int x = 0; x < superGridWidth; ++x) {
			superGrid[z][x].visited = false;
			superGrid[z][x].isPath = false;
		}
	}
}

void MapGenerator::PlaceBorderWalls()
{
	// 가장자리 1칸을 벽으로 설정 (이미 초기화에서 WALL이므로 유지)
	std::cout << "MapGenerator: Border walls placed" << std::endl;
}

void MapGenerator::PlaceStairs()
{
	// 4개 모서리 중 2개를 랜덤 선택
	std::vector<int> corners = { 0, 1, 2, 3 }; // 0:좌상, 1:우상, 2:좌하, 3:우하
	ShuffleVector(corners);

	int corner1 = corners[0];
	int corner2 = corners[1];

	// 계단 위치 결정 (2×2 타일, 모서리 근처)
	if (corner1 == 0 || corner2 == 0) {
		// 좌상단 2x2 (실제 맵에서 1~2, 1~2 위치)
		stair1Pos = glm::ivec2(1, 1);
		// 2x2 영역을 계단으로 설정
		for (int z = 1; z < 3 && z < mapHeight - 1; ++z) {
			for (int x = 1; x < 3 && x < mapWidth - 1; ++x) {
				map[z][x] = TileType::STAIR;
			}
		}
		std::cout << "MapGenerator: Stair1 (2x2) placed at [1,1]" << std::endl;
	}

	if (corner1 == 3 || corner2 == 3) {
		// 우하단 2x2
		int lastX = mapWidth - 2;
		int lastZ = mapHeight - 2;
		stair2Pos = glm::ivec2(lastX - 1, lastZ - 1);
		// 2x2 영역을 계단으로 설정
		for (int z = lastZ - 1; z <= lastZ && z < mapHeight - 1; ++z) {
			for (int x = lastX - 1; x <= lastX && x < mapWidth - 1; ++x) {
				map[z][x] = TileType::STAIR;
			}
		}
		std::cout << "MapGenerator: Stair2 (2x2) placed at [" << (lastX - 1) << "," << (lastZ - 1) << "]" << std::endl;
	}
	else if (corner1 == 1 || corner2 == 1) {
		// 우상단 2x2
		int lastX = mapWidth - 2;
		stair2Pos = glm::ivec2(lastX - 1, 1);
		// 2x2 영역을 계단으로 설정
		for (int z = 1; z < 3 && z < mapHeight - 1; ++z) {
			for (int x = lastX - 1; x <= lastX && x < mapWidth - 1; ++x) {
				map[z][x] = TileType::STAIR;
			}
		}
		std::cout << "MapGenerator: Stair2 (2x2) placed at [" << (lastX - 1) << ",1]" << std::endl;
	}
	else if (corner1 == 2 || corner2 == 2) {
		// 좌하단 2x2
		int lastZ = mapHeight - 2;
		stair2Pos = glm::ivec2(1, lastZ - 1);
		// 2x2 영역을 계단으로 설정
		for (int z = lastZ - 1; z <= lastZ && z < mapHeight - 1; ++z) {
			for (int x = 1; x < 3 && x < mapWidth - 1; ++x) {
				map[z][x] = TileType::STAIR;
			}
		}
		std::cout << "MapGenerator: Stair2 (2x2) placed at [1," << (lastZ - 1) << "]" << std::endl;
	}
}

void MapGenerator::GenerateMaze()
{
	std::cout << "MapGenerator: Generating 1-tile maze with DFS..." << std::endl;

	// 방문 기록용 2D 배열
	std::vector<std::vector<bool>> visited(mapHeight, std::vector<bool>(mapWidth, false));

	// 계단 영역을 방문 처리
	for (int z = 0; z < mapHeight; ++z) {
		for (int x = 0; x < mapWidth; ++x) {
			if (map[z][x] == TileType::STAIR) {
				visited[z][x] = true;
			}
		}
	}

	// 미로 시작 위치: 홀수 좌표에서 시작 (DFS 2칸 간격 알고리즘용)
	// 계단 근처의 홀수 좌표 찾기
	int mazeStartX = 3; // 홀수
	int mazeStartZ = 3; // 홀수

	std::cout << "MapGenerator: Maze starts from [" << mazeStartX << "," << mazeStartZ << "]" << std::endl;

	// 미로 생성
	DFS_Maze_SingleTile(mazeStartX, mazeStartZ, visited);

	// 계단에서 미로로 연결되는 입구 만들기
	// 계단 [1,1]~[2,2]에서 [3,3] 미로 시작점으로 연결
	map[3][2] = TileType::CORRIDOR; // 가로 연결
	map[2][3] = TileType::CORRIDOR; // 세로 연결
	std::cout << "MapGenerator: Created entrance from stairs to maze" << std::endl;

	std::cout << "MapGenerator: 1-tile maze generation complete" << std::endl;
}

void MapGenerator::DFS_Maze(int x, int z)
{
	// 1칸 단위 DFS 미로 생성 (재귀)
	static std::vector<std::vector<bool>> visited;
	if (visited.empty()) {
		visited.resize(mapHeight, std::vector<bool>(mapWidth, false));
	}

	DFS_Maze_SingleTile(x, z, visited);
}

void MapGenerator::DFS_Maze_SingleTile(int x, int z, std::vector<std::vector<bool>>& visited)
{
	// 범위 체크
	if (x < 0 || x >= mapWidth || z < 0 || z >= mapHeight) return;
	if (visited[z][x]) return;

	// 방문 표시
	visited[z][x] = true;

	// 현재 타일을 복도로 설정 (계단이 아닌 경우에만)
	if (map[z][x] != TileType::STAIR) {
		map[z][x] = TileType::CORRIDOR;
	}

	// 4방향 이웃 (상하좌우)
	std::vector<glm::ivec2> directions = {
		{0, -1},  // 위
		{0, 1},   // 아래
		{-1, 0},  // 왼쪽
		{1, 0}    // 오른쪽
	};
	ShuffleVector(directions);  // 랜덤 순서

	for (const auto& dir : directions) {
		int nx = x + dir.x * 2;  // 2칸 건너뛰기 (벽 1칸 + 통로 1칸)
		int nz = z + dir.y * 2;

		// 범위 체크 및 방문 여부 확인
		if (nx > 0 && nx < mapWidth - 1 && nz > 0 && nz < mapHeight - 1 && !visited[nz][nx]) {
			// 중간 타일을 복도로 만들기 (벽을 뚫음)
			int mx = x + dir.x;
			int mz = z + dir.y;
			if (map[mz][mx] != TileType::STAIR) {
				map[mz][mx] = TileType::CORRIDOR;
			}
			visited[mz][mx] = true;

			// 재귀 호출
			DFS_Maze_SingleTile(nx, nz, visited);
		}
	}
}

void MapGenerator::ExtractSinglePath()
{
	std::cout << "MapGenerator: Extracting single path with BFS..." << std::endl;

	BFS_FindPath();

	std::cout << "MapGenerator: Single path extracted" << std::endl;
}

void MapGenerator::BFS_FindPath()
{
	// BFS로 계단1 → 계단2 경로 찾기
	std::queue<glm::ivec2> q;
	std::vector<std::vector<glm::ivec2>> parent(superGridHeight, std::vector<glm::ivec2>(superGridWidth, glm::ivec2(-1, -1)));
	std::vector<std::vector<bool>> visited(superGridHeight, std::vector<bool>(superGridWidth, false));

	q.push(stair1Pos);
	visited[stair1Pos.y][stair1Pos.x] = true;

	bool found = false;

	while (!q.empty() && !found) {
		glm::ivec2 current = q.front();
		q.pop();

		if (current == stair2Pos) {
			found = true;
			break;
		}

		std::vector<glm::ivec2> neighbors = GetNeighbors(current.x, current.y);
		for (const auto& neighbor : neighbors) {
			int nx = neighbor.x;
			int nz = neighbor.y;

			if (!visited[nz][nx] && superGrid[nz][nx].visited) {  // DFS에서 방문한 타일만
				visited[nz][nx] = true;
				parent[nz][nx] = current;
				q.push(neighbor);
			}
		}
	}

	if (!found) {
		std::cerr << "ERROR: MapGenerator - Path not found between stairs!" << std::endl;
		return;
	}

	// 경로 역추적
	glm::ivec2 current = stair2Pos;
	while (current != glm::ivec2(-1, -1)) {
		superGrid[current.y][current.x].isPath = true;
		current = parent[current.y][current.x];
	}
}

void MapGenerator::FillRestWithWalls()
{
	std::cout << "MapGenerator: Filling non-path tiles with walls..." << std::endl;

	// 경로에 포함되지 않은 슈퍼타일을 벽으로 변환
	for (int z = 0; z < superGridHeight; ++z) {
		for (int x = 0; x < superGridWidth; ++x) {
			if (!superGrid[z][x].isPath) {
				// 경로가 아닌 슈퍼타일은 벽으로
				SetSuperTileArea(x, z, TileType::WALL);
			}
			else {
				// 경로인 슈퍼타일은 복도로 (계단 제외)
				int wx = superGrid[z][x].worldX;
				int wz = superGrid[z][x].worldZ;
				if (map[wz][wx] != TileType::STAIR) {
					SetSuperTileArea(x, z, TileType::CORRIDOR);
				}
			}
		}
	}
}

void MapGenerator::SetSuperTileArea(int gridX, int gridZ, TileType type)
{
	int worldX = superGrid[gridZ][gridX].worldX;
	int worldZ = superGrid[gridZ][gridX].worldZ;

	// 3×3 타일 블록 설정
	for (int dz = 0; dz < 3; ++dz) {
		for (int dx = 0; dx < 3; ++dx) {
			int wx = worldX + dx;
			int wz = worldZ + dz;
			if (wx >= 0 && wx < mapWidth && wz >= 0 && wz < mapHeight) {
				// 계단은 덮어쓰지 않음
				if (map[wz][wx] != TileType::STAIR) {
					map[wz][wx] = type;
				}
			}
		}
	}
}

bool MapGenerator::IsValidSuperTile(int gridX, int gridZ) const
{
	return gridX >= 0 && gridX < superGridWidth && gridZ >= 0 && gridZ < superGridHeight;
}

std::vector<glm::ivec2> MapGenerator::GetNeighbors(int gridX, int gridZ)
{
	std::vector<glm::ivec2> neighbors;

	// 상하좌우 4방향만 (대각선 제외)
	const int dx[] = { 0, 0, -1, 1 };
	const int dz[] = { -1, 1, 0, 0 };

	for (int i = 0; i < 4; ++i) {
		int nx = gridX + dx[i];
		int nz = gridZ + dz[i];
		if (IsValidSuperTile(nx, nz)) {
			neighbors.push_back(glm::ivec2(nx, nz));
		}
	}

	return neighbors;
}

void MapGenerator::ShuffleVector(std::vector<glm::ivec2>& vec)
{
	std::shuffle(vec.begin(), vec.end(), rng);
}

void MapGenerator::ShuffleVector(std::vector<int>& vec)
{
	std::shuffle(vec.begin(), vec.end(), rng);
}

TileType MapGenerator::GetTile(int x, int z) const
{
	if (x < 0 || x >= mapWidth || z < 0 || z >= mapHeight) {
		return TileType::WALL;
	}
	return map[z][x];
}

void MapGenerator::SetTile(int x, int z, TileType type)
{
	if (x >= 0 && x < mapWidth && z >= 0 && z < mapHeight) {
		map[z][x] = type;
	}
}

void MapGenerator::PrintMap() const
{
	std::cout << "\n===== Generated Map =====" << std::endl;
	for (int z = 0; z < mapHeight; ++z) {
		for (int x = 0; x < mapWidth; ++x) {
			switch (map[z][x]) {
			case TileType::WALL:      std::cout << "#"; break;
			case TileType::CORRIDOR:  std::cout << "H"; break;
			case TileType::STAIR:     std::cout << "S"; break;
			case TileType::CLASSROOM: std::cout << "C"; break;
			case TileType::DOOR:      std::cout << "D"; break;
			default:                  std::cout << "?"; break;
			}
		}
		std::cout << std::endl;
	}
	std::cout << "=========================" << std::endl;
}
