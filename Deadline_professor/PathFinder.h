#pragma once

#include "TotalHeader.h"

class NavMesh;
class NavNode;

// ========================================
// 경로 (노드들의 시퀀스)
// ========================================
struct Path
{
	std::vector<glm::vec3> waypoints;    // 월드 좌표 기반 경로점
	std::vector<NavNode*> nodeSequence;  // 네비게이션 노드 시퀀스
	bool isValid = false;                // 경로 유효성

	int GetCurrentWaypointIndex() const { return currentWaypointIndex; }
	void SetCurrentWaypointIndex(int index) { currentWaypointIndex = index; }

	glm::vec3 GetNextWaypoint() const
	{
		if (currentWaypointIndex < static_cast<int>(waypoints.size()))
		{
			return waypoints[currentWaypointIndex];
		}
		return glm::vec3(0.0f); // 경로 끝
	}

	bool IsComplete() const
	{
		return currentWaypointIndex >= static_cast<int>(waypoints.size());
	}

private:
	int currentWaypointIndex = 0;
};

// ========================================
// 경로 탐색 엔진
// ========================================
class PathFinder
{
public:
	PathFinder(NavMesh* navMesh);
	~PathFinder();

	// 경로 탐색
	// startPos, goalPos: 월드 좌표
	// goalDistance: 목표까지 도달해야 하는 최소 거리 (기본값: 0 = 정확히 도달)
	Path FindPath(const glm::vec3& startPos, const glm::vec3& goalPos, float goalDistance = 0.0f);

	// 경로 유효성 확인 및 재계산
	bool IsPathValid(const Path& path) const;
	Path RecalculatePath(const Path& currentPath, const glm::vec3& newStartPos);

	// NavMesh 업데이트
	void SetNavMesh(NavMesh* navMesh) { this->navMesh = navMesh; }
	NavMesh* GetNavMesh() const { return navMesh; }

private:
	NavMesh* navMesh = nullptr;

	// A* 알고리즘 호출
	bool ExecuteAStar(NavNode* startNode, NavNode* goalNode, std::vector<NavNode*>& outPath, float goalDistance);

	// 유틸리티
	float CalculateHeuristic(NavNode* from, NavNode* to) const;
	void CleanupNodeStates();
};
