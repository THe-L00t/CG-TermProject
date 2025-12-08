#pragma once

// ========================================
// 게임 전역 상수 (현실적인 단위 기반)
// 기본 단위: 1 unit = 1 meter
// ========================================

namespace GameConstants
{
	// ===== 플레이어 관련 =====
	constexpr float PLAYER_HEIGHT = 1.7f;           // 플레이어 키 (m)
	constexpr float PLAYER_WIDTH = 0.5f;            // 플레이어 폭 (m)
	constexpr float PLAYER_DEPTH = 0.5f;            // 플레이어 깊이 (m)
	constexpr float PLAYER_WALK_SPEED = 6.0f;       // 걷기 속도 (m/s) - 약 5 km/h
	//constexpr float PLAYER_WALK_SPEED = 50.0f;       // 걷기 속도 (m/s) - 약 5 km/h
	constexpr float PLAYER_RUN_SPEED = 5.0f;        // 뛰기 속도 (m/s) - 약 18 km/h
	constexpr float PLAYER_EYE_HEIGHT = 1.5f;       // 플레이어 눈 높이 (m)

	// ===== 손전등 조명 관련 ⭐⭐⭐ =====
	constexpr float FLASHLIGHT_INTENSITY = 1.0f;           // 손전등 밝기
	constexpr float FLASHLIGHT_INNER_CUTOFF = 8.0f;       // ⭐ 내부 각도
	constexpr float FLASHLIGHT_OUTER_CUTOFF = 12.0f;      // ⭐ 외부 각도
	constexpr float FLASHLIGHT_RANGE = 30.0f;             // 손전등 범위 (m)
	constexpr float FLASHLIGHT_OFFSET_FORWARD = 0.2f;     // ⭐ 카메라에서 앞으로 오프셋 (0.3f → 0.2f)
	constexpr float FLASHLIGHT_OFFSET_DOWN = 0.3f;        // ⭐ 카메라에서 아래로 오프셋 (0.15f → 0.3f, 손 높이)

	// ===== 교수님 관련 =====
	constexpr float PROFESSOR_MODEL_SCALE = 1.0f;   // FBX 모델 스케일 보정값 (모델이 작을 경우 이 값을 조정)
	constexpr float PROFESSOR_MOVE_SPEED = 4.5f;    // 교수님 이동 속도 (m/s) - 플레이어보다 약간 빠름
	constexpr float PROFESSOR_DETECTION_RANGE = 15.0f; // 교수님 감지 범위 (m)
	constexpr float PROFESSOR_COLLISION_RADIUS = 1.5f; // 교수님과의 충돌 거리

	// ===== 환경 관련 =====
	constexpr float FLOOR_DEFAULT_WIDTH = 100.0f;    // 바닥 기본 너비 (m) - 2배 확장
	constexpr float FLOOR_DEFAULT_HEIGHT = 100.0f;   // 바닥 기본 깊이 (m) - 2배 확장
	constexpr float CEILING_HEIGHT = 2.5f;          // 천장 높이 (m)
	constexpr float WALL_THICKNESS = 0.1f;          // 벽 두께 (m)
	constexpr float CEILING_TEXTURE_TILE_X = 20.0f;  // 천장 텍스처 타일링 X축 반복 횟수
	constexpr float CEILING_TEXTURE_TILE_Y = 20.0f;  // 천장 텍스처 타일링 Y축 반복 횟수
	constexpr float FLOOR_TEXTURE_TILE_X = 20.0f;    // 바닥 텍스처 타일링 X축 반복 횟수
	constexpr float FLOOR_TEXTURE_TILE_Y = 20.0f;    // 바닥 텍스처 타일링 Y축 반복 횟수

	// ===== 맵/타일 관련 =====
	constexpr float TILE_SIZE = 4.0f;               // 한 타일의 그리드 간격 (m) - 벽 중심 간 거리
	constexpr float WALL_SIZE = 2.f;               // 벽 객체의 실제 크기 (m) - 인접 벽과 0.2m 간격
	constexpr float WALL_HEIGHT = 3.f;             // 벽 높이 (m) - 바닥부터 천장까지
	constexpr int MAP_GRID_WIDTH = 25;              // 맵 그리드 가로 크기 (타일 개수) - 100m / 4m = 25
	constexpr int MAP_GRID_DEPTH = 25;              // 맵 그리드 세로 크기 (타일 개수) - 100m / 4m = 25

	// ===== 카메라 관련 =====
	constexpr float CAMERA_FOV = 60.0f;             // 시야각 (degrees)
	constexpr float CAMERA_NEAR_PLANE = 0.1f;       // Near plane (m)
	constexpr float CAMERA_FAR_PLANE = 100.0f;      // Far plane (m)
	constexpr float CAMERA_SENSITIVITY = 0.1f;      // 마우스 감도
	constexpr float CAMERA_VERTICAL_SPEED_MULTIPLIER = 0.3f;	// 수직 이동 속도 보정값

	// ===== 물리 관련 =====
	constexpr float GRAVITY = 9.81f;                // 중력 가속도 (m/s²)
	constexpr float COLLISION_EPSILON = 0.01f;      // 충돌 허용 오차 (m)

	// ===== 깜빡이는 조명 관련 ⭐⭐⭐ =====
	constexpr float FLICKERING_LIGHT_HEIGHT = 2.3f;      // 천장보다 약간 아래 (천장 2.5m)
	constexpr int FLICKERING_LIGHT_MIN_SPACING = 8;      // 최소 간격 (타일 단위) - 32m
	constexpr float FLICKERING_LIGHT_INTENSITY = 2.5f;   // 기본 밝기
	constexpr int FLICKERING_LIGHT_MAX_COUNT = 20;       // 맵당 최대 개수

}
