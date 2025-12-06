#include "Plane.h"
#include "GameConstants.h"

Plane::Plane()
	: Object(),
	orientation(Orientation::UP),
	size(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT)
{
	// 기본값: 바닥으로 설정 (위를 향함)
	// 기본 크기: 현실적인 크기 (20m x 20m)
	SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT);
}

Plane::~Plane()
{
}

void Plane::SetOrientation(Orientation newOrientation)
{
	orientation = newOrientation;
	needsUpdate = true;

	// Orientation에 따라 자동으로 회전값 설정 (도 단위로 저장)
	switch (orientation)
	{
	case Orientation::UP:
		// 바닥: 기본 방향 (Y축 위를 향함)
		SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		break;

	case Orientation::DOWN:
		// 천장: X축으로 180도 회전
		SetRotation(glm::vec3(180.0f, 0.0f, 0.0f));
		break;

	case Orientation::FRONT:
		// 앞면: X축으로 -90도 회전
		SetRotation(glm::vec3(-90.0f, 0.0f, 0.0f));
		break;

	case Orientation::BACK:
		// 뒷면: X축으로 90도 회전
		SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
		break;

	case Orientation::LEFT:
		// 왼쪽: Z축으로 90도 회전
		SetRotation(glm::vec3(0.0f, 0.0f, 90.0f));
		break;

	case Orientation::RIGHT:
		// 오른쪽: Z축으로 -90도 회전
		SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
		break;
	}
}

Plane::Orientation Plane::GetOrientation() const
{
	return orientation;
}

void Plane::SetSize(float width, float height)
{
	size.x = width;
	size.y = height;

	// Scale을 통해 크기 반영
	// 기본 평면 메쉬가 1x1이라고 가정
	SetScale(glm::vec3(width, 1.0f, height));
	needsUpdate = true;
}

glm::vec2 Plane::GetSize() const
{
	return size;
}

void Plane::Update(float deltaTime)
{
	// 부모 클래스의 Update 호출
	Object::Update(deltaTime);

	// Plane 특화 업데이트 로직이 필요하면 여기에 추가
}
