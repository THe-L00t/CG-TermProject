#include "Wall.h"
#include "GameConstants.h"

Wall::Wall()
	: Object(),
	gridPosition(0, 0),
	tileSize(GameConstants::WALL_SIZE, GameConstants::WALL_HEIGHT, GameConstants::WALL_SIZE)
{
	// 기본 벽 크기 설정 (WALL_SIZE: 벽 객체 실제 크기)
	SetTileSize(GameConstants::WALL_SIZE, GameConstants::WALL_SIZE, GameConstants::WALL_HEIGHT);

	// 기본 리소스 설정
	SetResourceID("CubeModel");
	SetTextureID("WallTexture");
	SetColor(glm::vec3(1.0f, 1.0f, 1.0f)); // 흰색 (텍스처 원본 색상)
}

Wall::~Wall()
{
}

void Wall::SetTileSize(float tileWidth, float tileDepth, float height)
{
	tileSize = glm::vec3(tileWidth, height, tileDepth);

	// Scale을 통해 크기 반영
	// Cube 메쉬가 1x1x1이라고 가정
	SetScale(glm::vec3(tileWidth, height, tileDepth));
	needsUpdate = true;
}

void Wall::SetGridPosition(int gridX, int gridZ)
{
	gridPosition = glm::ivec2(gridX, gridZ);

	// 그리드 위치를 월드 좌표로 변환
	// 벽의 중심이 타일 중앙에 오도록 설정
	// 맵의 중심을 원점(0,0,0)으로 만들기 위해 오프셋 적용
	float halfMapSize = (GameConstants::MAP_GRID_WIDTH * GameConstants::TILE_SIZE) * 0.5f;
	float worldX = (gridX * GameConstants::TILE_SIZE) - halfMapSize + (GameConstants::TILE_SIZE * 0.5f);
	float worldY = tileSize.y * 0.5f; // 벽 높이의 절반 (바닥 기준)
	float worldZ = (gridZ * GameConstants::TILE_SIZE) - halfMapSize + (GameConstants::TILE_SIZE * 0.5f);

	SetPosition(glm::vec3(worldX, worldY, worldZ));
}

glm::ivec2 Wall::GetGridPosition() const
{
	return gridPosition;
}

void Wall::Update(float deltaTime)
{
	// 부모 클래스의 Update 호출
	Object::Update(deltaTime);

	// Wall 특화 업데이트 로직이 필요하면 여기에 추가
}
