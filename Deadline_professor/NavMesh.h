#pragma once

#include "TotalHeader.h"
#include "MapGenerator.h"

// ========================================
// 네비게이션 노드 (한 타일 = 한 노드)
// ========================================ㅇ
class NavNode
{
public:
	NavNode(int gridX, int gridZ, bool walkable = true);

	// 기본 정보
	int GetGridX() const { return gridX; }
	int GetGridZ() const { return gridZ; }
	bool IsWalkable() const { return walkable; }
	void SetWalkable(bool value) { walkable = value; }

	// 월드 좌표 계산 (타일 중심)
	glm::vec3 GetWorldPosition() const;

	// 인접 노드
	void AddNeighbor(NavNode* neighbor) { neighbors.push_back(neighbor); }
	const std::vector<NavNode*>& GetNeighbors() const { return neighbors; }
	void ClearNeighbors() { neighbors.clear(); }

	// A* 알고리즘용 (경로 탐색 시마다 초기화 필요)
	float GetGCost() const { return gCost; }
	float GetHCost() const { return hCost; }
	float GetFCost() const { return gCost + hCost; }
	void SetGCost(float value) { gCost = value; }
	void SetHCost(float value) { hCost = value; }

	NavNode* GetParent() const { return parent; }
	void SetParent(NavNode* node) { parent = node; }

	// 노드 상태
	enum class State { NONE, OPEN, CLOSED };
	State GetState() const { return state; }
	void SetState(State s) { state = s; }

private:
	int gridX, gridZ;           // 그리드 좌표
	bool walkable;              // 이동 가능 여부
	std::vector<NavNode*> neighbors; // 인접 노드 (최대 4개: 상하좌우)

	// A* 알고리즘 관련
	float gCost = 0.0f;         // 시작점으로부터의 비용
	float hCost = 0.0f;         // 목표점까지의 휴리스틱 비용
	NavNode* parent = nullptr;  // 부모 노드 (경로 추적)
	State state = State::NONE;  // 노드 상태
};

// ========================================
// 네비게이션 메시 (타일 기반 그래프)
// ========================================
class NavMesh
{
public:
	NavMesh(int gridWidth, int gridDepth);
	~NavMesh();

	// 초기화 (맵 데이터 기반 메시 생성)
	void Initialize(const std::vector<std::vector<TileType>>& mapData);

	// 노드 접근
	NavNode* GetNode(int gridX, int gridZ) const;
	NavNode* GetNodeFromWorldPos(const glm::vec3& worldPos) const;

	// 메시 정보
	int GetGridWidth() const { return gridWidth; }
	int GetGridDepth() const { return gridDepth; }
	const std::vector<std::unique_ptr<NavNode>>& GetAllNodes() const { return nodes; }

	// 타일 크기
	float GetTileSize() const;

	// 인접 검증 (NPC 충돌 영역 기반)
	bool IsWalkableArea(int gridX, int gridZ) const;
	bool CanMoveBetween(int fromX, int fromZ, int toX, int toZ) const;

private:
	int gridWidth, gridDepth;                   // 그리드 크기
	std::vector<std::unique_ptr<NavNode>> nodes; // 모든 노드 (1D 배열로 관리)

	// 노드 관리
	NavNode* GetNodeInternal(int index) const;
	int CoordToIndex(int gridX, int gridZ) const;
	void CoordFromIndex(int index, int& gridX, int& gridZ) const;

	// 인접 노드 관계 설정
	void ConnectNeighbors();

	// NPC 충돌 영역 검증
	bool IsCollisionFree(int centerX, int centerZ) const;
};
