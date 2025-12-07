#pragma once

#include "TotalHeader.h"
#include <queue>

class NavNode;

// ========================================
// A* 알고리즘 구현
// ========================================
class AStarAlgorithm
{
public:
	AStarAlgorithm();
	~AStarAlgorithm();

	// A* 탐색 실행
	// goalDistance: 목표까지 도달해야 하는 최소 거리
	// 반환: 경로 찾기 성공 여부
	bool Search(NavNode* startNode, NavNode* goalNode, std::vector<NavNode*>& outPath, float goalDistance = 0.0f);

private:
	// 비교 함수 (우선순위 큐용)
	struct CompareNode
	{
		bool operator()(NavNode* a, NavNode* b) const;
	};

	// 유틸리티
	float CalculateHeuristic(NavNode* from, NavNode* to) const;
	float CalculateDistance(NavNode* from, NavNode* to) const;
	void ReconstructPath(NavNode* currentNode, std::vector<NavNode*>& outPath) const;
};
