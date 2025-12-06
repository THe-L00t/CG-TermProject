#pragma once
#include "TotalHeader.h"
#include <vector>
#include <queue>
#include <stack>
#include <random>

// 맵 타일 타입
enum class TileType
{
	WALL,       // # 벽
	CORRIDOR,   // H 복도
	STAIR,      // S 계단
	CLASSROOM,  // C 강의실
	DOOR        // D 문
};

// 슈퍼타일 구조체 (3x3 타일 블록)
struct SuperTile
{
	int gridX, gridZ;           // 슈퍼타일 그리드 좌표
	int worldX, worldZ;         // 월드 타일 좌표 (실제 맵에서의 시작 위치)
	bool visited;               // DFS 방문 여부
	bool isPath;                // 최종 경로 포함 여부

	SuperTile() : gridX(0), gridZ(0), worldX(0), worldZ(0), visited(false), isPath(false) {}
	SuperTile(int gx, int gz, int wx, int wz)
		: gridX(gx), gridZ(gz), worldX(wx), worldZ(wz), visited(false), isPath(false) {}
};

// 맵 생성 클래스
class MapGenerator
{
public:
	MapGenerator(int width, int depth);
	~MapGenerator();

	// 메인 맵 생성 함수
	void Generate();

	// 맵 데이터 접근
	TileType GetTile(int x, int z) const;
	void SetTile(int x, int z, TileType type);
	const std::vector<std::vector<TileType>>& GetMap() const { return map; }

	// 디버그 출력
	void PrintMap() const;

private:
	// 맵 크기
	int mapWidth;
	int mapHeight;
	int superGridWidth;
	int superGridHeight;

	// 맵 데이터
	std::vector<std::vector<TileType>> map;           // 실제 타일 맵
	std::vector<std::vector<SuperTile>> superGrid;    // 슈퍼타일 그리드

	// 계단 위치
	glm::ivec2 stair1Pos;
	glm::ivec2 stair2Pos;

	// 랜덤 생성기
	std::mt19937 rng;

	// 생성 단계별 함수
	void InitializeMap();
	void PlaceBorderWalls();
	void PlaceStairs();
	void GenerateMaze();
	void ExtractSinglePath();
	void FillRestWithWalls();

	// DFS 미로 생성
	void DFS_Maze(int gridX, int gridZ);
	void DFS_Maze_SingleTile(int x, int z, std::vector<std::vector<bool>>& visited);

	// BFS 경로 추출
	void BFS_FindPath();

	// 유틸리티 함수
	bool IsValidSuperTile(int gridX, int gridZ) const;
	void SetSuperTileArea(int gridX, int gridZ, TileType type);
	std::vector<glm::ivec2> GetNeighbors(int gridX, int gridZ);
	void ShuffleVector(std::vector<glm::ivec2>& vec);
	void ShuffleVector(std::vector<int>& vec);
};
