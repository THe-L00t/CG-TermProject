# CG-TermProject
# Deadline:교수님

**컴퓨터 그래픽스 최종 프로젝트**  
OpenGL 3.3 기반 1인칭 공포 호러 술래잡기 게임

---

## 프로젝트 정보

- **과목**: 컴퓨터 그래픽스
- **개발 기간**: 약 4주 (2025.11.11 ~ 2025.12.07)
- **개발 인원**: 2명
- **개발 환경**: Windows 10/11, OpenGL 3.3 Core, C++20
- **총 커밋 수**: 168개
- **현재 진행률**: 약 70%

### 팀원
- 이태형 (2024182028)
- 민현규 (2024180014)

---

## 게임 소개

### 개요
3층 규모의 학교 건물에서 3명의 게임공학과 교수님들을 피해 탈출하는 1인칭 공포 호러 게임입니다.

### 개발 동기
이전 프로젝트 *KICK THE PROFESSOR*의 좋은 반응을 바탕으로 후속작을 기획하게 되었습니다. 아쉬웠던 부분을 개선하고 장르를 공포 호러로 변경했습니다.

### 게임 컨셉
- **장르**: 1인칭 공포 호러 술래잡기
- **참고 게임**: 데드 바이 데이라이트 (공포 분위기, 추격 메커니즘)
- **핵심 메커니즘**: 술래잡기

---

## 주요 기능

### 구현 완료 기능
- [x] 기본 게임 프레임워크 (Engine, Window, Renderer)
- [x] 엄격한 클래스 책임 분리 아키텍처
- [x] 리소스 매니저 시스템 (FBX, OBJ, PNG 로더)
- [x] 셰이더 시스템 (Phong Lighting)
- [x] 씬 관리 시스템 (Title, Floor1~3, Test)
- [x] 1인칭/3인칭 카메라 시스템
- [x] 입력 관리 시스템 (키보드, 마우스)
- [x] 게임 타이머 (델타타임 기반)
- [x] FBX 모델 및 애니메이션 로딩 (Assimp)
- [x] PNG 텍스처 로딩 (LodePNG)
- [x] 조명 시스템 (Directional, Point, Spot)
- [x] 플레이어 캐릭터 클래스
- [x] 교수 캐릭터 클래스 (AI 추격 기초)
- [x] 프로젝트 구조 정리 (Resources, Textures, Shaders 폴더)

### 구현 예정 기능
- [ ] 뷰포트 응용 미니맵
- [ ] 셰이더 기반 공포 화면 효과
- [ ] 3층 규모 학교 맵 완성
- [ ] 충돌 감지 시스템
- [ ] 사운드 시스템
- [ ] UI 시스템 완성
- [ ] AI 추격 로직 고도화
- [ ] 레벨 디자인 및 최적화

### 기술 스택
- **그래픽스 API**: OpenGL 3.3 Core Profile
- **셰이더**: GLSL (Vertex/Fragment Shaders)
- **모델 로딩**: Assimp (FBX 지원)
- **텍스처 로딩**: LodePNG
- **윈도우 관리**: FreeGLUT
- **수학 라이브러리**: GLM
- **빌드 환경**: Visual Studio 2022, C++20 (stdcpplatest)

---

## 개발 일정

| 주차 | 기간 | 목표 | 상태 |
|:---:|---|---|:---:|
| 1주차 | 11.11~11.17 | 기본 프레임워크 구축 | ✅ |
| 2주차 | 11.18~11.24 | 리소스 시스템 구현 | ✅ |
| 3주차 | 11.25~12.01 | 카메라 및 입력 시스템 | ✅ |
| 4주차 | 12.02~12.05 | 게임플레이 통합 | ✅ |
| 5주차 | 12.06~12.07 | 최종 마무리 및 제출 | 🚧 |

---

## 프로젝트 구조

```
CG-TermProject/
├── Deadline_professor/          # 메인 프로젝트 디렉토리
│   ├── Resources/               # 3D 모델 파일
│   │   ├── RunLee.fbx          # FBX 애니메이션 모델
│   │   ├── RunDragon.fbx
│   │   ├── RunSong.fbx
│   │   └── *.obj               # OBJ 모델 파일
│   ├── Textures/                # 텍스처 이미지
│   │   ├── RunLee.png
│   │   └── *.png
│   ├── Shaders/                 # 셰이더 파일
│   │   ├── basic.vert           # 버텍스 셰이더
│   │   └── basic.frag           # 프래그먼트 셰이더
│   ├── assimp/                  # Assimp 라이브러리
│   ├── Engine.h/cpp             # 메인 엔진 클래스
│   ├── Renderer.h/cpp           # 렌더링 시스템
│   ├── ResourceManager.h/cpp    # 리소스 관리
│   ├── SceneManager.h/cpp       # 씬 관리
│   ├── Camera.h/cpp             # 카메라 시스템
│   ├── InputManager.h/cpp       # 입력 처리
│   ├── GameTimer.h/cpp          # 타이머
│   ├── Object.h/cpp             # 오브젝트 베이스
│   ├── Player.h/cpp             # 플레이어
│   ├── Professor.h/cpp          # 교수 NPC
│   ├── Light.h/cpp              # 조명
│   ├── FBXAnimationPlayer.h/cpp # 애니메이션
│   ├── LoadPng.h/cpp            # PNG 로더
│   ├── Shader.h/cpp             # 셰이더
│   ├── Window.h/cpp             # 윈도우
│   ├── TotalHeader.h            # 통합 헤더
│   ├── PROJECT_RULES.md         # 아키텍처 문서
│   └── main.cpp                 # 엔트리 포인트
└── README.md
```

### 아키텍처 원칙

**엄격한 클래스 책임 분리**:
- 입력 처리 → **InputManager만**
- 렌더링 → **Renderer만**
- 리소스 로딩 → **ResourceManager만**
- 씬 관리 → **SceneManager만**

자세한 내용은 [PROJECT_RULES.md](Deadline_professor/PROJECT_RULES.md) 참조

### 주요 클래스
- **Engine**: 게임 루프, 초기화, 모든 시스템 통합
- **Window**: GLUT 윈도우 생성 및 이벤트
- **Renderer**: OpenGL 렌더링, 셰이더 관리
- **ResourceManager**: FBX/OBJ/PNG 로딩 및 GPU 버퍼 관리
- **SceneManager**: 씬 전환 (Factory Pattern)
- **Camera**: 1인칭/3인칭, View/Projection Matrix
- **InputManager**: 키보드/마우스 (함수 객체 기반)
- **GameTimer**: 델타타임 계산
- **Object**: Transform 기반 베이스 클래스
- **Player**: 플레이어 캐릭터 (카메라 연동)
- **Professor**: 교수 NPC (AI, 애니메이션)
- **Light**: 조명 (Directional, Point, Spot)
- **FBXAnimationPlayer**: 스켈레탈 애니메이션

---

## 개발 로그

### 1주차 (2025.11.11 ~ 2025.11.17) - 완료
#### 2025.11.11
- [x] 프로젝트 초기 설정 (Visual Studio 프로젝트 생성)
- [x] C++ 버전 및 링커 설정
- [x] main.cpp 생성
- [x] README 작성

#### 2025.11.12
- [x] Window 클래스 생성 및 구현 (GLUT 기반)
  - Create 함수 구현
  - Resize 콜백 함수 구현
  - 싱글 인스턴스 패턴 적용
- [x] Engine 클래스 생성
  - Initialize 함수 정의
  - 콜백 함수 등록 구조 설계
- [x] Shader 클래스 생성
- [x] 통합 헤더 구성 (TotalHeader.h)

#### 2025.11.13
- [x] 통합 헤더 업데이트 (optional, filesystem 등)
- [x] C++ 버전 수정 및 필터 정보 설정

#### 2025.11.14
- [x] Shader 클래스 함수 구현
  - LoadFile, AddShader, CompileShader 정의
  - Use/Unuse 인터페이스 추가

#### 2025.11.15
- [x] Shader setUniform 함수 오버로딩 (다양한 타입 지원)
- [x] 기본 셰이더 파일 생성 (test.vs, test.fs)
- [x] shaderList 필터 생성
- [x] .gitignore 설정

#### 2025.11.17
- [x] Shader glCreateProgram 수정

### 2주차 (2025.11.18 ~ 2025.11.24) - 완료
#### 2025.11.18
- [x] Shader getAttrib 함수 추가

#### 2025.11.19
- [x] ResourceManager 클래스 생성
  - 필요 구조체 선언 (Vertex, ObjData)
  - 복사/이동 생성자/할당연산자 구현
  - LoadObj 함수 임시 완성

#### 2025.11.21
- [x] Renderer 클래스 구성 시작
  - Shader를 unordered_map으로 관리
  - ResourceManager 단일 객체로 관리
- [x] Window 클래스 개선
  - width/height getter 추가
  - 함수 객체를 통한 resize 콜백
  - Viewport 관련 기능 Renderer로 이동
- [x] ResourceManager SortData 함수 추가
- [x] Window/Renderer 병합 테스트
- [x] 통합 헤더에 unordered_map 추가

#### 2025.11.22
- [x] GameTimer 클래스 구현
- [x] Engine-Renderer 콜백 구조 완성
- [x] 테스트용 기본 버텍스/프래그먼트 셰이더 수정
- [x] Shader 유니폼 함수 const 수정
- [x] 테스트 완료

#### 2025.11.23
- [x] SceneManager 클래스 구현
- [x] Scene 기반 클래스 구조 설계
- [x] Scene 자식 클래스 선언
- [x] 씬 전환 시스템 구현
- [x] 현재 씬 함수 호출 완료

#### 2025.11.24
- [x] ResourceManager 일부 수정
- [x] OBJ 파일 로더 개선
  - v, v/vt, v/vt/vn, v//vn 형식 파싱 지원
  - Triangle/Quad face 지원
  - Fan triangulation 구현
  - Vector out of range 에러 수정
- [x] ResourceManager를 Engine으로 이동 (에셋 사전 로드)
- [x] OBJ 로더 최적화 (삼각형 face 직접 처리)
- [x] 고폴리곤 모델 테스트 (bugatti.obj - 744k vertices)

### 3주차 (2025.11.25 ~ 2025.12.01) - 완료
#### 2025.11.25
- [x] Camera 클래스 생성
- [x] 기본 구조 설계

#### 2025.11.26
- [x] Camera 코딩 시작
  - View Matrix 계산 구현
  - Projection Matrix 계산 구현
- [x] 통합 헤더 불러오기

#### 2025.11.27
- [x] Camera 클래스 완성
  - 1인칭/3인칭 모드 지원
  - 마우스 회전 처리
  - InputManager 연결 준비
- [x] InputManager 클래스 구현
  - 키보드 입력 처리
  - 함수 객체 기반 액션 바인딩 (ActionW, ActionA, ActionS, ActionD 등)
  - 마우스 컨트롤 지원
- [x] 통합 헤더 병합 전 저장

#### 2025.11.28
- [x] 에셋 추가 및 테스트
- [x] 기본 렌더링 테스트 완료
- [x] 포맷 갱신

#### 2025.11.29
- [x] Assimp 라이브러리 설정
- [x] FBX 로딩 시스템 구현 시작
- [x] 애니메이션 데이터 구조 설계
- [x] FBX 로딩 디버깅

#### 2025.11.30
- [x] Assimp 행렬 변환 문제 해결 시도
- [x] 본 변환 행렬 계산 디버깅

#### 2025.12.01
- [x] FBX 로딩 계속 디버깅
- [x] 행렬 변환 문제 분석

### 4주차 (2025.12.02 ~ 2025.12.05) - 완료
#### 2025.12.03
- [x] FBX 애니메이션 로딩 **최종 성공**!
- [x] 본 변환 행렬 계산 완료
- [x] 다양한 FBX 모델 테스트 (RunLee, RunDragon, RunSong)
- [x] 행렬 변환 문제 완전 해결

#### 2025.12.04
- [x] Assimp 세팅 완료 및 안정화
- [x] Object 베이스 클래스 구현
  - Transform 정보 관리 (Position, Rotation, Scale)
  - Model Matrix 계산
  - 리소스 ID 관리
- [x] Light 클래스 구현 완성
  - Directional Light (방향광)
  - Point Light (점광원)
  - Spot Light (스포트라이트)
  - ApplyToShader 메서드로 셰이더 연동
- [x] Professor 클래스 구현
  - FBX 모델 연결 및 로딩
  - 애니메이션 재생 시스템
  - 기본 AI 추격 로직 (플레이어 추적)
  - Update, Draw 메서드
- [x] Player 클래스 구현
  - Camera 연동 (1인칭/3인칭 전환)
  - 이동 로직 (MoveForward, MoveBackward, MoveLeft, MoveRight)
  - InputManager와 연결
- [x] SceneManager 씬 분리
  - TitleScene: 타이틀 화면
  - Floor1Scene: 1층 게임플레이 (Player, Professor 배치)
  - Floor2Scene, Floor3Scene: 2~3층 (준비 단계)
  - TestScene: 테스트용 씬
- [x] 조명 시스템 병합
- [x] Professor 클래스 병합
- [x] Player 클래스 병합
- [x] 모든 기능 통합 및 테스트
- [x] 기본 게임플레이 구조 완성

#### 2025.12.05 (오늘)
- [x] **프로젝트 파일 구조 대규모 정리**
  - Resources 폴더 생성 및 .fbx, .obj 파일 이동
  - Textures 폴더 생성 및 .png 파일 이동
  - Shaders 폴더 생성 및 .vert, .frag 파일 이동
  - Visual Studio 프로젝트 파일 업데이트 (모든 리소스 필터에 추가)
- [x] **텍스처 로딩 시스템 개선**
  - stb_image 문제 발견 (placeholder 파일로 구현 안 됨)
  - LodePNG 라이브러리로 완전 전환
  - LoadPng.h/cpp 프로젝트에 추가
  - ResourceManager 텍스처 로딩 코드 리팩토링
  - RGBA 포맷으로 통일, 불필요한 코드 제거
- [x] **리소스 경로 업데이트**
  - 셰이더 로드 경로: `Shaders/basic.vert`, `Shaders/basic.frag`
  - 텍스처 로드 경로: `Textures/RunLee.png`
  - FBX 모델 로드 경로: `Resources/RunLee.fbx`
- [x] **디버그 단축키 추가**
  - 숫자 1~5: 각 씬으로 전환 (Title, Floor1~3, Test)
  - 숫자 0: 마우스 카메라 컨트롤 ON/OFF 토글
  - InputManager에 Action0~Action5 함수 객체 추가
- [x] **코드 정리 및 문서화**
  - stb_image.h 파일 완전 삭제
  - 불필요한 디버그 코드 제거
  - 주석 정리 및 개선
  - README.md 전체 갱신 (168개 커밋 기록 반영)

---

## 변경 사항

개발 과정에서 발생한 주요 변경 사항을 기록합니다.

### 2025.12.05
- 프로젝트 파일 구조 대규모 정리 (Resources, Textures, Shaders 폴더 분리)
- 텍스처 로딩 시스템 개선 (stb_image → LodePNG)
- 디버그 단축키 추가 (씬 전환 1~5, 마우스 컨트롤 0)
- 코드 정리 및 README 대폭 갱신

### 2025.12.04
- Object, Light, Professor, Player 클래스 통합
- 씬별 분리 완료 (Title, Floor1~3, Test)
- 기본 게임플레이 구조 완성

### 2025.12.03
- FBX 로딩 및 애니메이션 재생 성공
- Assimp 행렬 변환 문제 해결

### 2025.11.27
- Camera 시스템 완성 (1인칭/3인칭)
- InputManager 구현 완료 (함수 객체 기반)

### 2025.11.24
- ResourceManager 개선 (다양한 OBJ face 형식 지원)
- Engine으로 리소스 관리 이동

### 2025.11.23
- SceneManager 및 Scene 시스템 구현

### 2025.11.22
- GameTimer 통합
- Engine-Renderer 콜백 구조 완성

### 2025.11.21
- Renderer 클래스 본격 구현

### 2025.11.19
- ResourceManager 및 OBJ 로더 구현

### 2025.11.14~15
- Shader 클래스 완성

### 2025.11.12
- Window, Engine 기본 구조 구축

### 2025.11.11
- 프로젝트 시작

---

## 라이선스

본 프로젝트는 컴퓨터 그래픽스 과목의 학술 목적으로 개발되었습니다.
