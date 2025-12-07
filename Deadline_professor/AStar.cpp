#include "AStar.h"
#include "NavMesh.h"

AStarAlgorithm::AStarAlgorithm()
{
}

AStarAlgorithm::~AStarAlgorithm()
{
}

bool AStarAlgorithm::Search(NavNode* startNode, NavNode* goalNode, std::vector<NavNode*>& outPath, float goalDistance)
{
	if (!startNode || !goalNode)
	{
		std::cerr << "ERROR: AStarAlgorithm - Start or goal node is null" << std::endl;
		return false;
	}

	// 오픈 리스트 (우선순위 큐: F 비용이 작은 순서대로)
	std::priority_queue<NavNode*, std::vector<NavNode*>, CompareNode> openList;
	std::vector<NavNode*> closedList;

	// 시작 노드 초기화
	startNode->SetGCost(0.0f);
	startNode->SetHCost(CalculateHeuristic(startNode, goalNode));
	startNode->SetState(NavNode::State::OPEN);
	openList.push(startNode);

	while (!openList.empty())
	{
		// 1단계: F 비용이 가장 낮은 노드 선택
		NavNode* currentNode = openList.top();
		openList.pop();

		// 2단계: 목표 도달 확인
		float distToGoal = CalculateDistance(currentNode, goalNode);
		if (distToGoal <= goalDistance)
		{
			std::cout << "Goal reached!" << std::endl;
			ReconstructPath(currentNode, outPath);
			return true;
		}

		// 3단계: 현재 노드를 클로즈 리스트로 이동
		currentNode->SetState(NavNode::State::CLOSED);
		closedList.push_back(currentNode);

		// 4단계: 인접 노드 탐색
		const auto& neighbors = currentNode->GetNeighbors();
		for (NavNode* neighbor : neighbors)
		{
			if (!neighbor || !neighbor->IsWalkable())
				continue;

			// 클로즈 리스트에 이미 있으면 스킵
			if (neighbor->GetState() == NavNode::State::CLOSED)
				continue;

			// 새로운 G 비용 계산 (현재 노드를 거쳐 온 경우)
			float tentativeGCost = currentNode->GetGCost() + CalculateDistance(currentNode, neighbor);

			// 오픈 리스트에 있는 경우
			if (neighbor->GetState() == NavNode::State::OPEN)
			{
				// 더 나은 경로를 찾으면 업데이트
				if (tentativeGCost >= neighbor->GetGCost())
					continue;
			}

			// 새 경로가 더 좋거나 처음 만난 노드
			neighbor->SetParent(currentNode);
			neighbor->SetGCost(tentativeGCost);
			neighbor->SetHCost(CalculateHeuristic(neighbor, goalNode));
			neighbor->SetState(NavNode::State::OPEN);
			openList.push(neighbor);
		}
	}

	std::cout << "WARNING: AStarAlgorithm - No path found to goal" << std::endl;
	return false;
}

bool AStarAlgorithm::CompareNode::operator()(NavNode* a, NavNode* b) const
{
	// F 비용(G + H)이 작을수록 높은 우선순위 (최소 힙)
	// priority_queue는 내림차순이므로, 역으로 비교
	return a->GetFCost() > b->GetFCost();
}

float AStarAlgorithm::CalculateHeuristic(NavNode* from, NavNode* to) const
{
	if (!from || !to)
		return 0.0f;

	glm::vec3 fromPos = from->GetWorldPosition();
	glm::vec3 toPos = to->GetWorldPosition();

	// 맨해튼 거리 (휴리스틱 함수)
	// 그리드 기반 이동이므로 맨해튼 거리가 적절
	float dx = std::abs(fromPos.x - toPos.x);
	float dz = std::abs(fromPos.z - toPos.z);
	return dx + dz;
}

float AStarAlgorithm::CalculateDistance(NavNode* from, NavNode* to) const
{
	if (!from || !to)
		return 0.0f;

	glm::vec3 fromPos = from->GetWorldPosition();
	glm::vec3 toPos = to->GetWorldPosition();

	// 유클리드 거리 (실제 이동 비용)
	return glm::distance(fromPos, toPos);
}

void AStarAlgorithm::ReconstructPath(NavNode* currentNode, std::vector<NavNode*>& outPath) const
{
	outPath.clear();

	NavNode* node = currentNode;
	while (node != nullptr)
	{
		outPath.push_back(node);
		node = node->GetParent();
	}

	// 경로 역순 (시작점 → 목표점)
	std::reverse(outPath.begin(), outPath.end());
}
