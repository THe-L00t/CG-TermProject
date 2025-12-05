#pragma once
#include "TotalHeader.h"
#include "Object.h"

// 벽 오브젝트를 표현하는 클래스
// Object를 상속받아 기본 Transform 기능 사용
// n*m 타일 기반 맵에서 하나의 벽 타일을 표현
class Wall : public Object
{
public:
	Wall();
	virtual ~Wall();

	// 벽의 크기 설정 (타일 크기 기반)
	void SetTileSize(float tileWidth, float tileDepth, float height);

	// 벽 타일의 그리드 위치 설정
	void SetGridPosition(int gridX, int gridZ);
	glm::ivec2 GetGridPosition() const;

	virtual void Update(float deltaTime) override;

private:
	glm::ivec2 gridPosition; // 그리드 상의 위치 (x, z)
	glm::vec3 tileSize;      // 타일 크기 (width, height, depth)
};
