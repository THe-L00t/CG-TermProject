#include "PathFinder.h"
#include "NavMesh.h"
#include "AStar.h"
#include "GameConstants.h"

PathFinder::PathFinder(NavMesh* navMesh)
	: navMesh(navMesh)
{
	if (!navMesh)
	{
		std::cout << "WARNING: PathFinder created with null NavMesh" << std::endl;
	}
}

PathFinder::~PathFinder()
{
}

Path PathFinder::FindPath(const glm::vec3& startPos, const glm::vec3& goalPos, float goalDistance)
{
	Path result;
	result.isValid = false;

	if (!navMesh)
	{
		std::cerr << "ERROR: PathFinder - NavMesh is null" << std::endl;
		return result;
	}

	// 1단계: 월드 좌표 → 그리드 좌표 변환
	NavNode* startNode = navMesh->GetNodeFromWorldPos(startPos);
	NavNode* goalNode = navMesh->GetNodeFromWorldPos(goalPos);

	// ⭐ 디버깅 추가
	std::cout << "\n===== PathFinder DEBUG =====" << std::endl;
	std::cout << "Start position: (" << startPos.x << ", " << startPos.y << ", " << startPos.z << ")" << std::endl;
	std::cout << "Goal position: (" << goalPos.x << ", " << goalPos.y << ", " << goalPos.z << ")" << std::endl;

	if (startNode) {
		std::cout << "Start node: Grid[" << startNode->GetGridX() << ", " << startNode->GetGridZ() << "]" << std::endl;
		std::cout << "Start node walkable: " << (startNode->IsWalkable() ? "YES" : "NO") << std::endl;
		std::cout << "Start node world pos: (" << startNode->GetWorldPosition().x << ", "
			<< startNode->GetWorldPosition().y << ", " << startNode->GetWorldPosition().z << ")" << std::endl;
	}
	else {
		std::cout << "Start node: NULL" << std::endl;
	}

	if (goalNode) {
		std::cout << "Goal node: Grid[" << goalNode->GetGridX() << ", " << goalNode->GetGridZ() << "]" << std::endl;
		std::cout << "Goal node walkable: " << (goalNode->IsWalkable() ? "YES" : "NO") << std::endl;
	}
	else {
		std::cout << "Goal node: NULL" << std::endl;
	}
	std::cout << "============================\n" << std::endl;

	if (!startNode || !startNode->IsWalkable())
	{
		std::cerr << "ERROR: PathFinder - Start position is not walkable" << std::endl;
		return result;
	}

	if (!goalNode || !goalNode->IsWalkable())
	{
		std::cerr << "ERROR: PathFinder - Goal position is not walkable" << std::endl;
		return result;
	}

	// 2단계: A* 알고리즘 실행
	std::vector<NavNode*> nodePath;
	bool pathFound = ExecuteAStar(startNode, goalNode, nodePath, goalDistance);

	if (!pathFound || nodePath.empty())
	{
		std::cout << "WARNING: PathFinder - No path found" << std::endl;
		return result;
	}

	// 3단계: 노드 경로 → 월드 좌표 경로 변환
	result.nodeSequence = nodePath;
	for (const auto& node : nodePath)
	{
		result.waypoints.push_back(node->GetWorldPosition());
	}

	result.isValid = true;
	result.SetCurrentWaypointIndex(0);

	std::cout << "✓ Path found: " << nodePath.size() << " waypoints" << std::endl;

	return result;
}

bool PathFinder::IsPathValid(const Path& path) const
{
	if (!path.isValid || path.nodeSequence.empty())
	{
		return false;
	}

	// 모든 노드가 여전히 이동 가능한지 확인
	for (const auto& node : path.nodeSequence)
	{
		if (!node || !node->IsWalkable())
		{
			return false;
		}
	}

	return true;
}

Path PathFinder::RecalculatePath(const Path& currentPath, const glm::vec3& newStartPos)
{
	if (currentPath.nodeSequence.size() < 2)
	{
		return FindPath(newStartPos, currentPath.waypoints.back());
	}

	// 현재 경로의 마지막 목표 사용
	return FindPath(newStartPos, currentPath.waypoints.back());
}

bool PathFinder::ExecuteAStar(NavNode* startNode, NavNode* goalNode, std::vector<NavNode*>& outPath, float goalDistance)
{
	if (!startNode || !goalNode)
	{
		return false;
	}

	// 이전 탐색의 노드 상태 초기화
	CleanupNodeStates();

	// A* 알고리즘 실행 (클래스 이름과 메서드 이름이 다름)
	AStarAlgorithm astarAlgorithm;
	return astarAlgorithm.Search(startNode, goalNode, outPath, goalDistance);
}

float PathFinder::CalculateHeuristic(NavNode* from, NavNode* to) const
{
	if (!from || !to)
	{
		return 0.0f;
	}

	glm::vec3 fromPos = from->GetWorldPosition();
	glm::vec3 toPos = to->GetWorldPosition();

	// 맨해튼 거리 (휴리스틱)
	float dx = std::abs(fromPos.x - toPos.x);
	float dz = std::abs(fromPos.z - toPos.z);
	return dx + dz;
}

void PathFinder::CleanupNodeStates()
{
	if (!navMesh)
		return;

	const auto& allNodes = navMesh->GetAllNodes();
	for (const auto& node : allNodes)
	{
		if (node)
		{
			node->SetState(NavNode::State::NONE);
			node->SetParent(nullptr);
			node->SetGCost(0.0f);
			node->SetHCost(0.0f);
		}
	}
}
