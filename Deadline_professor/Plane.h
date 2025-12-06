#pragma once
#include "TotalHeader.h"
#include "Object.h"

// 바닥, 천장 등 평면 오브젝트를 표현하는 클래스
// Object를 상속받아 기본 Transform 기능 사용
class Plane : public Object
{
public:
	Plane();
	virtual ~Plane();

	// 평면의 방향 설정 (예: 바닥은 위를 향함, 천장은 아래를 향함)
	enum class Orientation
	{
		UP,    // 바닥 (Y축 양의 방향을 향함)
		DOWN,  // 천장 (Y축 음의 방향을 향함)
		FRONT, // 앞면
		BACK,  // 뒷면
		LEFT,  // 왼쪽
		RIGHT  // 오른쪽
	};

	void SetOrientation(Orientation orientation);
	Orientation GetOrientation() const;

	// 평면의 크기 설정 (width, height)
	void SetSize(float width, float height);
	glm::vec2 GetSize() const;

	virtual void Update(float deltaTime) override;

private:
	Orientation orientation;
	glm::vec2 size; // width, height
};
