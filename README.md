# CG-TermProject
# Deadline:교수님

**컴퓨터 그래픽스 최종 프로젝트**  
OpenGL 3.3 기반 1인칭 공포 호러 술래잡기 게임

---

## 프로젝트 정보

- **과목**: 컴퓨터 그래픽스
- **개발 기간**: 약 4주 (2025.11.11 ~ 2025.12.07)
- **개발 인원**: 2명
- **개발 환경**: Windows 10/11, OpenGL 3.3, C++
- **현재 진행률**: 약 50%

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

### 구현 예정 기능
- [x] 기본 게임 프레임워크 (Engine, Window, Renderer)
- [x] 리소스 매니저 시스템 (OBJ 로더 포함)
- [x] 셰이더 시스템 (기본 라이팅 지원)
- [x] 씬 매니저
- [x] 게임 타이머
- [ ] 1인칭 카메라 시스템
- [ ] 뷰포트 응용 미니맵
- [ ] 셰이더 기반 공포 화면 효과
- [ ] 3층 규모 학교 맵
- [ ] 맵 실시간 로드 시스템
- [ ] 술래(교수님) 추격 AI

### 기술 스택
- **그래픽스 API**: OpenGL 3.3
- **셰이더**: GLSL
- **프레임워크**: 2D 게임 프로그래밍 수업 기반

---

## 개발 일정

| 주차 | 목표 | 상태 |
|:---:|---|:---:|
| 1주차 (11.11~11.17) | 프로젝트 설정, 기본 클래스 구조 설계 | ✅ 완료 |
| 2주차 (11.18~11.24) | 게임 구성에 필요한 클래스 제작 | ✅ 완료 |
| 3주차 (11.25~12.01) | 리소스 로드, 게임 로직, 셰이더 효과 구현 | ⬜ 예정 |
| 4주차 (12.02~12.07) | AI 구현, 오류 수정 및 제출 준비 | ⬜ 예정 |

---

## 프로젝트 구조

```
CG-TermProject/
├── Deadline_professor/       # 메인 프로젝트 디렉토리
│   ├── Engine.h/cpp         # 게임 엔진 코어
│   ├── Window.h/cpp         # 윈도우 관리 (GLUT)
│   ├── Renderer.h/cpp       # 렌더링 시스템
│   ├── ResourceManager.h/cpp # 리소스 관리 (OBJ 로더)
│   ├── Shader.h/cpp         # 셰이더 관리
│   ├── SceneManager.h/cpp   # 씬 관리
│   ├── GameTimer.h/cpp      # 타이머 시스템
│   ├── TotalHeader.h        # 공통 헤더
│   ├── main.cpp             # 엔트리 포인트
│   ├── basic.vert           # 기본 버텍스 셰이더
│   ├── basic.frag           # 기본 프래그먼트 셰이더
│   └── *.obj                # 3D 모델 파일
└── README.md
```

### 주요 클래스
- **Engine**: 게임 루프, 초기화, 콜백 관리
- **Window**: GLUT 기반 윈도우 생성 및 이벤트 처리
- **Renderer**: OpenGL 렌더링 파이프라인
- **ResourceManager**: OBJ 파일 로딩 및 GPU 버퍼 관리
- **Shader**: GLSL 셰이더 컴파일 및 유니폼 관리
- **SceneManager**: 게임 씬 전환 및 관리
- **GameTimer**: 프레임 타이밍 및 델타타임 계산

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

### 3주차 (2025.11.25 ~ 2025.12.01)
- [ ] 작업 내용 추가 예정

### 4주차 (2025.12.02 ~ 2025.12.07)
- [ ] 작업 내용 추가 예정

---

## 변경 사항

개발 과정에서 발생한 주요 변경 사항을 기록합니다.

### 2025.11.24
- ResourceManager를 Renderer에서 Engine으로 이동
  - 창 생성 직후 에셋 사전 로드 방식으로 변경
  - 리소스 관리 중앙화
- OBJ 파일 로더 개선
  - 다양한 face 형식 지원 추가 (v, v/vt, v/vt/vn, v//vn)
  - Quad face 자동 삼각형 변환 (fan triangulation)
  - Vector out of range 에러 수정
  - 삼각형 face 직접 처리로 성능 최적화

### 2025.11.23
- SceneManager 시스템 구현
- Scene 기반 아키텍처 구축

### 2025.11.22
- GameTimer 시스템 통합
- Engine-Renderer 콜백 구조 완성
- 통합 테스트 완료

### 2025.11.21
- Renderer 클래스 본격 구현
- Window 클래스 개선 (함수 객체 콜백)
- Viewport 관련 기능 분리

### 2025.11.19
- ResourceManager 클래스 생성 및 OBJ 로더 구현

### 2025.11.14~15
- Shader 클래스 핵심 기능 구현
- setUniform 함수 오버로딩

### 2025.11.12
- Window, Engine, Shader 클래스 생성
- 통합 헤더 구성

### 2025.11.11
- 프로젝트 시작

---

## 라이선스

본 프로젝트는 컴퓨터 그래픽스 과목의 학술 목적으로 개발되었습니다.
