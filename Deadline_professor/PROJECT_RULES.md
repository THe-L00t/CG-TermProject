# 프로젝트 규칙 및 아키텍처

## 기본 사양

- **그래픽스 API**: OpenGL 3.3
- **언어**: C++ (latest 버전)
- **에셋 로딩**: Assimp를 사용한 FBX 로드 및 애니메이션 재생

## 아키텍처 개요

이 프로젝트는 각 클래스가 명확하게 정의된 역할을 가지며, 그 경계를 절대 넘지 않는 엄격한 클래스 책임 아키텍처를 따릅니다.

## 클래스 책임

### Engine
- 모든 클래스들을 관리하고 초기화
- 전체 시스템을 총괄
- **특정 도메인 로직(입력, 렌더링, 리소스 등)은 처리하지 않음**

### InputManager
- **모든** 입력 처리는 이 클래스에서만 수행
- 다른 클래스는 입력 이벤트를 직접 처리하지 않음
- 다른 시스템에 입력 상태를 제공

### Renderer
- **모든** 그리기 작업은 이 클래스에서만 수행
- 셰이더 객체들을 내부적으로 관리
- 모든 렌더링 파이프라인 작업 처리
- 다른 클래스는 OpenGL draw 함수를 직접 호출하지 않음
- **다중 조명 시스템 관리** (최대 8개 동시 지원):
  - `AddLight(Light*)`: 렌더링 시스템에 조명 추가
  - `RemoveLight(Light*)`: 특정 조명 제거
  - `ClearLights()`: 모든 조명 제거
  - `GetLights()`: 등록된 조명 목록 반환
  - `ApplyLightsToShader(Shader*)`: 모든 활성 조명을 셰이더에 전달
  - `RenderLightDebugPoints()`: 디버그용 조명 위치 시각화 (흰색 점으로 표시)
- **주요 렌더링 함수**:
  - `RenderObj()`: OBJ 모델 렌더링 (색상 지원)
  - `RenderObjWithTexture()`: 텍스처가 적용된 OBJ 모델 렌더링
  - `RenderObjWithTextureTiled()`: 텍스처 타일링이 적용된 OBJ 렌더링
  - `RenderFBX()`: 텍스처가 적용된 FBX 모델 렌더링
  - `RenderFBXAnimated()`: 스켈레탈 애니메이션 + 텍스처 렌더링
- **조명 통합**: 모든 렌더링 함수는 자동으로 다중 조명 시스템을 적용
  - 렌더링 함수 호출 시 `ApplyLightsToShader()` 자동 호출
  - 레거시 단일 조명(`SetLight()`)과 하위 호환성 유지
  - 다중 조명이 없을 때 기본 조명 자동 사용

### ResourceManager
- **모든** 리소스 로딩은 이 클래스에서만 관리
- 로드된 모든 리소스(모델, 텍스처, 사운드 등)를 저장 및 관리
- 다른 시스템에 리소스 접근 제공
- 리소스 생명주기 관리 (로딩, 캐싱, 언로딩)
- **FBX 로딩 설정** (확정 - 수정 금지):
  - Assimp 플래그: `aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_FlipWindingOrder`
  - **aiProcess_FlipWindingOrder**: FBX의 CW(Clockwise) 와인딩 오더를 OpenGL의 CCW(Counter-Clockwise)로 변환 (백페이스 컬링과 호환)
  - **텍스처 V축 플립**: `1.0f - mesh->mTextureCoords[0][i].y` (OpenGL 텍스처 좌표계에 맞춤)
  - 이 설정들은 검증 완료되었으므로 절대 변경하지 말 것

### SceneManager
- **모든** 게임 씬을 관리
- 씬 전환 제어
- 현재 활성화된 씬 유지

### Scene (베이스/개별 씬)
- 각 씬은 자신의 객체들을 소유
- 씬별 객체 관리
- Update와 Draw 메서드를 통해 씬별 로직과 렌더링 수행
- **씬은 직접 렌더링하지 않고, Renderer를 통해 그리기 요청**

#### 씬 예시:
- **TitleScene**: 객체가 없거나 최소한의 UI 객체만 포함 가능
- **Floor1Scene** (게임플레이 씬): 필수 포함:
  - Player 객체
  - Professor 객체 (추격 대상)
  - 기타 게임플레이 관련 객체들
- **Floor2Scene, Floor3Scene**: 각 층별 게임플레이 씬
- **TestScene**: 개발/테스트용 씬

### Camera
- 카메라 시점 관리
- 뷰 행렬(View Matrix)과 투영 행렬(Projection Matrix) 제공
- 카메라 이동 및 회전 처리
- Renderer에 행렬 정보 제공

### Shader
- 셰이더 프로그램 생성 및 컴파일
- Uniform 변수 설정
- **Renderer에 의해 관리됨**
- 다른 클래스는 Shader를 직접 생성하거나 관리하지 않음
- **다중 조명 셰이더 시스템** (basic.vert / basic.frag):
  - **Vertex Shader (basic.vert)**:
    - 스켈레탈 애니메이션 지원 (최대 100개 본)
    - 본 가중치 기반 정점 변환
    - Normal 변환 행렬 계산
  - **Fragment Shader (basic.frag)**:
    - 최대 8개 조명 동시 처리
    - 조명 타입별 계산 함수:
      - `CalculateDirectionalLight()`: 방향성 조명 계산
      - `CalculatePointLight()`: 점광원 + 거리 감쇠 계산
      - `CalculateSpotLight()`: 스포트라이트 + 원뿔 강도 + 감쇠 계산
    - Phong 조명 모델 (Ambient + Diffuse + Specular)
    - 텍스처 타일링 지원 (`uTextureTiling`)
    - 하위 호환성: `uLightCount == 0`일 때 레거시 조명 사용
- **셰이더 프로그램 접근**: `GetProgram()` 메서드로 프로그램 ID 반환

### Object (베이스 클래스)
- 게임 내 모든 오브젝트의 기본 클래스
- **주요 기능**:
  - 위치, 회전, 크기 등의 Transform 정보 관리
  - 모델 행렬(Model Matrix) 계산
  - 리소스 ID를 통해 ResourceManager의 리소스 참조
  - 텍스처 ID 및 타일링 정보 관리
    - `SetTextureID()` / `GetTextureID()`: 텍스처 리소스 ID 관리
    - `SetTextureTiling()` / `GetTextureTiling()`: 텍스처 타일링 설정 (기본값: 1x1)
  - 색상(Color) 정보 관리
  - 활성화/비활성화 상태 관리
- **렌더링은 Renderer에 위임**

### Player
- Object를 상속받은 플레이어 캐릭터 클래스
- 플레이어 이동 로직 처리
- Camera와 연동하여 1인칭 시점 제어
- **입력은 InputManager에서 받아옴**

### Professor
- Object를 상속받은 교수 캐릭터 클래스
- 플레이어를 추격하는 AI 로직 처리
- 애니메이션 재생 제어
- 이동 및 방향 전환 로직

### Light
- Object를 상속받은 조명 클래스
- **3가지 조명 타입 지원**:
  - **DIRECTIONAL (방향성 조명)**: 태양광처럼 무한히 먼 곳에서 비추는 평행광
    - `SetDirection()`: 빛의 방향 설정 (위치가 아닌 방향 벡터)
    - 감쇠(Attenuation) 없음
  - **POINT (점광원)**: 모든 방향으로 빛을 발산하는 전구형 조명
    - `SetPosition()`: 조명의 위치 설정
    - `SetAttenuation()`: 거리에 따른 밝기 감쇠 설정 (상수, 선형, 이차)
    - 감쇠 공식: `1.0 / (constant + linear × distance + quadratic × distance²)`
  - **SPOT (스포트라이트)**: 원뿔 모양으로 빛을 비추는 손전등형 조명
    - `SetPosition()`: 조명의 위치 설정
    - `SetDirection()`: 빛이 향하는 방향 설정
    - `SetSpotAngle()` / `SetCutOff()`: 내부 각도와 외부 경계 각도 설정 (코사인 값)
    - `SetAttenuation()`: 거리에 따른 감쇠 설정
- **조명 속성 관리**:
  - `SetAmbient()`: 주변광 색상 (항상 존재하는 최소 밝기)
  - `SetDiffuse()`: 확산광 색상 (표면에 직접 닿는 빛의 색)
  - `SetSpecular()`: 반사광 색상 (반짝이는 하이라이트 색)
  - `SetIntensity()`: 조명 강도 (0.0 ~ 무제한, 기본 1.0)
  - `SetEnabled()`: 조명 활성화/비활성화
- **셰이더 통합**:
  - `ApplyToShader(shaderProgram, lightIndex)`: 지정된 인덱스의 조명 유니폼에 데이터 전달
  - Phong 조명 모델 기반 계산 (Ambient + Diffuse + Specular)
- **다중 조명 시스템**: Renderer에 등록하여 최대 8개 조명 동시 사용 가능

### Plane
- Object를 상속받은 평면 오브젝트 클래스
- **역할**: 바닥, 천장, 벽 등의 평면 환경 요소를 표현
- **주요 기능**:
  - Orientation 설정을 통해 방향 제어 (UP, DOWN, FRONT, BACK, LEFT, RIGHT)
  - 크기(width, height) 설정 기능
  - 텍스처 적용 및 타일링(Tiling) 지원
    - `SetTextureID()`: 평면에 적용할 텍스처 지정
    - `SetTextureTiling()`: 텍스처 반복 횟수 설정 (X, Y축)
  - Player/Professor가 이동할 수 있는 바닥 레벨 제공
- **렌더링**: Renderer의 `RenderFBXModelWithTextureTiled()` 함수를 통해 타일링된 텍스처 렌더링
- **사용 예시**:
  - 바닥: Orientation::UP, FloorTexture 적용
  - 천장: Orientation::DOWN, CeilingTexture 적용
  - 벽: Orientation::FRONT/BACK/LEFT/RIGHT
- **설정 관리**: 타일링 반복 횟수는 GameConstants에서 조절 가능

### Wall
- Object를 상속받은 벽 오브젝트 클래스
- **역할**: n×m 타일 기반 맵에서 벽 타일을 표현하는 육면체 오브젝트
- **주요 기능**:
  - 타일 크기 설정: `SetTileSize(width, depth, height)`
  - 그리드 위치 설정: `SetGridPosition(gridX, gridZ)`
    - 그리드 좌표를 월드 좌표로 자동 변환
  - 텍스처 적용: 윗면과 아랫면을 제외한 4면에 텍스처 적용
  - 텍스처 ID는 Object 클래스의 `SetTextureID()` 사용
- **크기 설정**:
  - 가로/세로: `TILE_SIZE` (GameConstants)
  - 높이: `WALL_HEIGHT` (바닥부터 천장까지)
- **맵 구조**:
  - 맵은 `MAP_GRID_WIDTH × MAP_GRID_DEPTH` 크기의 그리드
  - 각 타일은 벽(Wall) 또는 공간(Empty)
- **렌더링**: Cube 메쉬(1×1×1) 기반, Scale을 통해 크기 조정
- **충돌**: 플레이어/교수님과의 충돌 처리에 사용

### GameTimer
- 게임 시간 관리
- 델타타임(DeltaTime) 계산
- 프레임 독립적인 게임 로직 지원

### FBXAnimationPlayer
- FBX 애니메이션 재생 관리
- 본(Bone) 변환 행렬 계산
- 애니메이션 타임라인 제어
- Renderer에 본 행렬 정보 제공

### Window
- GLUT/FreeGLUT 윈도우 생성 및 관리
- 윈도우 이벤트 처리
- OpenGL 컨텍스트 초기화

## OpenGL 렌더링 설정 (확정 - 수정 금지)

### 백페이스 컬링
- **기본 상태**: `glEnable(GL_CULL_FACE)` - 활성화 (Engine.cpp line 52)
- **Plane 렌더링 시**:
  - 바닥/천장 렌더링 **직전**에 `glDisable(GL_CULL_FACE)` 호출
  - 렌더링 **직후**에 `glEnable(GL_CULL_FACE)` 호출하여 복원
  - **위치**: SceneManager.cpp의 모든 Scene Draw 함수 (Floor1, Floor2, Floor3, Test)
- **이유**:
  - Plane 객체(바닥/천장)는 와인딩 오더 문제로 양면 렌더링 필요
  - FBX 모델(Professor, Player)은 성능을 위해 백페이스 컬링 유지
  - OBJ Plane과 OpenGL CCW 설정 간의 호환성 보장
- **절대 변경 금지**: Plane 렌더링 시 백페이스 컬링을 비활성화하지 않으면 바닥/천장이 보이지 않음

### 깊이 테스트
- **상태**: `glEnable(GL_DEPTH_TEST)` - 활성화
- **위치**: Engine.cpp 초기화 (line 51)
- **필수 유지**: 3D 렌더링의 올바른 깊이 처리를 위해 반드시 활성화

## 핵심 규칙

1. **역할 무결성**: 각 클래스는 **오직** 자신에게 지정된 역할만 수행해야 합니다. 이 분리는 **절대적**이며 반드시 유지되어야 합니다.

2. **역할 교차 금지**:
   - 입력 처리 → InputManager에서만
   - 그리기 → Renderer에서만
   - 리소스 로딩 → ResourceManager에서만
   - 씬 관리 → SceneManager에서만

3. **셰이더 관리**: 셰이더는 Renderer 클래스 내부에서 관리됩니다.

4. **리소스 관리**: 로드된 모든 리소스는 ResourceManager에서 추적 및 관리되어야 합니다.

5. **씬 소유권**: 객체들은 각자의 씬에 속하며, 전역 매니저에 속하지 않습니다 (핵심 시스템 제외).

## 코딩 표준

### 파일 인코딩
- **모든 소스 파일은 UTF-8 인코딩을 사용해야 합니다**
- `.h`, `.hpp`, `.cpp` 파일 모두 UTF-8 인코딩 준수
- BOM(Byte Order Mark) 포함 여부는 프로젝트 설정에 따름

### 임시 파일 관리
- **테스트용으로 생성한 파일은 사용 후 반드시 삭제**
- 삭제 대상 파일:
  - Python 테스트 파일 (`.py`)
  - NUL 파일
  - 임시 로그 파일
  - 기타 테스트/디버깅용 임시 파일
- 프로젝트 디렉토리를 깨끗하게 유지
- 커밋 전 불필요한 파일이 없는지 확인

## 다중 조명 시스템 가이드

### 조명 설정 예시

```cpp
// 1. Directional Light (태양광)
auto dirLight = std::make_unique<Light>(LightType::DIRECTIONAL);
dirLight->SetDirection(glm::vec3(-0.2f, -1.0f, -0.3f));
dirLight->SetAmbient(glm::vec3(0.2f, 0.2f, 0.25f));
dirLight->SetDiffuse(glm::vec3(0.5f, 0.5f, 0.6f));
dirLight->SetSpecular(glm::vec3(0.3f, 0.3f, 0.3f));
dirLight->SetIntensity(0.8f);
dirLight->SetEnabled(true);

// 2. Point Light (전구)
auto pointLight = std::make_unique<Light>(LightType::POINT);
pointLight->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));
pointLight->SetAmbient(glm::vec3(0.1f, 0.1f, 0.1f));
pointLight->SetDiffuse(glm::vec3(1.0f, 0.9f, 0.8f));
pointLight->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
pointLight->SetIntensity(1.5f);
pointLight->SetAttenuation(1.0f, 0.09f, 0.032f);  // 약 50m 범위
pointLight->SetEnabled(true);

// 3. Spot Light (손전등)
auto spotLight = std::make_unique<Light>(LightType::SPOT);
spotLight->SetPosition(glm::vec3(0.0f, 2.0f, 0.0f));
spotLight->SetDirection(glm::vec3(0.0f, -0.8f, -0.6f));
spotLight->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));
spotLight->SetDiffuse(glm::vec3(1.0f, 1.0f, 0.9f));
spotLight->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
spotLight->SetIntensity(2.0f);
spotLight->SetAttenuation(1.0f, 0.09f, 0.032f);
spotLight->SetSpotAngle(
    glm::cos(glm::radians(12.5f)),  // 내부 각도
    glm::cos(glm::radians(17.5f))   // 외부 각도
);
spotLight->SetEnabled(true);

// Renderer에 등록
renderer->AddLight(dirLight.get());
renderer->AddLight(pointLight.get());
renderer->AddLight(spotLight.get());
```

### 감쇠(Attenuation) 거리 참고표

| Constant | Linear | Quadratic | 유효 범위 |
|----------|--------|-----------|----------|
| 1.0 | 0.7 | 1.8 | ~7m |
| 1.0 | 0.35 | 0.44 | ~13m |
| 1.0 | 0.22 | 0.20 | ~20m |
| 1.0 | 0.14 | 0.07 | ~30m |
| 1.0 | 0.09 | 0.032 | ~50m |
| 1.0 | 0.07 | 0.017 | ~70m |
| 1.0 | 0.045 | 0.0075 | ~100m |

### 주의사항

- **최대 조명 개수**: 8개 (셰이더 `MAX_LIGHTS` 제한)
- **성능**: 조명이 많을수록 프래그먼트 셰이더 연산 증가
- **조명 우선순위**: 먼저 추가된 조명부터 처리
- **하위 호환성**: 레거시 `SetLight()` 사용 시 다중 조명 무시됨

## 개발 가이드라인

- 새로운 기능을 추가할 때는 항상 어느 클래스가 해당 도메인을 책임지는지 확인
- 편의를 위해 지정된 매니저 클래스를 우회하지 말 것
- 클래스 책임 간의 명확한 경계 유지
- 책임이 불명확한 경우, 이 문서를 참조하거나 팀과 논의
