# AI 시스템 개발 체크리스트 (간소화 버전)

## 🎯 프로젝트 목표
- **목표**: 맵 기반 NPC 경로 탐색 기술 시연
- **핵심**: A* 알고리즘 + NavMesh + 경로 추격
- **범위**: 기본 기능만 (고급 AI 기능 제외)

---

## ✅ 완료된 기능

### 기초 시스템
- [x] NavMesh 시스템 (노드 기반 그래프)
- [x] NavMeshBuilder (맵 데이터 → NavMesh 변환)
- [x] A* 경로 탐색 알고리즘
- [x] PathFinder 경로 탐색 엔진
- [x] AIController 기본 구조
- [x] Path 구조 및 waypoint 시스템

---

## 🚧 필수 구현 작업 (우선순위순)

### Phase 1: AIController 핵심 기능 ✅ 완료

#### 1-1. UpdateMovement 메서드 구현 ✅

목표: NPC가 경로를 따라 이동하도록 구현

- [x] Waypoint 도달 판정
  - 현재 waypoint와 NPC 위치 거리 계산
  - 도달 거리 임계값 설정 (0.5m)
  - 다음 waypoint로 진행

- [x] 경로 완료 판정
  - 마지막 waypoint 도달 시 완료 처리
  - IDLE로 전환

- [x] 실제 이동 적용
  - GetNextMoveDirection()으로 이동 방향 구하기
  - PROFESSOR_MOVE_SPEED (5.5 m/s) 적용
  - NPC 위치 업데이트

#### 1-2. GetNextMoveDirection 메서드 구현 ✅

목표: 현재→다음 waypoint로 향하는 방향 벡터 반환

- [x] 현재 waypoint 조회
  - Path.GetNextWaypoint() 사용
  - 경로가 없으면 vec3(0,0,0) 반환

- [x] 방향 벡터 계산
  - 다음 waypoint - 현재 위치
  - glm::normalize() 정규화

- [x] 속도 벡터 변환
  - 방향만 반환 (외부에서 속도 곱함)

#### 1-3. ChaseTarget 메서드 구현 ⏸️ 보류

- [x] 구현 완료 (현재 게임 요구사항에 맞게 비활성화)
- [ ] 나중에 필요시 활용 (주석 처리됨)

#### 1-4. ValidateAndUpdatePath 메서드 구현 ✅

목표: 경로가 여전히 유효한지 확인

- [x] 경로 유효성 확인
  - PathFinder::IsPathValid(currentPath) 호출

- [x] 경로 무효 시 재계산
  - 새 경로 계산
  - STUCK 상태 처리

- [x] 호출 시기
  - 0.5초마다 주기적 갱신

#### 1-5. HasReachedTarget 메서드 구현 ✅

- [x] Path 완료 여부 확인
  - currentPath.IsComplete() 반환

---

### Phase 2: Professor 클래스 연동 ✅ 완료

#### 2-1. Professor에서 AIController 사용 ✅

- [x] AIController 멤버 변수 추가
  - `AIController* aiController{nullptr};`
  - `glm::vec3 patrolTarget{0.0f};`

- [x] Professor::Update()에서 AIController 업데이트
  - SetCurrentPosition()으로 현재 위치 전달
  - 플레이어 감지 범위 확인
  - SetTargetPosition() 또는 ClearTarget() 호출
  - UpdateMovement() 호출

- [x] 이동 적용
  - GetNextMoveDirection() 으로 방향 벡터 획득
  - Position 업데이트
  - direction 업데이트 (애니메이션용)

- [x] 새 메서드 추가
  - SetAIController(AIController* controller)
  - GetAIController() const
  - SetPatrolTarget(const glm::vec3& targetPos)

#### 2-2. NPC 3명 각각 AIController 할당 (준비됨)

- [ ] Lee 교수님: AIController 인스턴스 1
- [ ] Dragon 교수님: AIController 인스턴스 2
- [ ] Song 교수님: AIController 인스턴스 3
- [ ] 각 AIController는 동일한 NavMesh/PathFinder 공유
- 📌 **상태**: Scene에서 구현 필요

---

### Phase 3: 기본 테스트 및 검증 🚧 진행 중

#### 3-1. 단일 NPC 경로 추적 테스트

- [ ] TestSence에서 NPC가 플레이어를 피해 이동하는지 확인
  - 경로 이동이 자연스러운지
  - waypoint 도달이 정확한지
  - 목표 도달 시 멈추는지

#### 3-2. 경로 없음 테스트

- [ ] 막힌 공간에서 NPC가 가만히 있는지 확인
- [ ] STUCK 상태에서 움직이지 않는지 확인

#### 3-3. 다중 NPC 테스트

- [ ] 3명의 교수님이 동시에 도망치는지 확인
- [ ] 경로 계산 중복 최소화 확인 (FPS 영향 없는지)

#### 3-4. 맵별 테스트

- // Test에서 정상 작동시
- [ ] Floor1, Floor2, Floor3 각각 경로 추적 확인
- [ ] 각 층의 벽/복도 구조에서 경로가 정확한지 확인

---

## 📊 구현 순서 (최적화)
Step 1: GetNextMoveDirection 구현 (10분) ↓ 
Step 2: UpdateMovement 구현 (20분) ↓ 
Step 3: ChaseTarget 구현 (15분) ↓ 
Step 4: ValidateAndUpdatePath 구현 (10분) ↓ 
Step 5: Professor 클래스에 AIController 연동 (15분) ↓ 
Step 6: 테스트 및 디버깅 (30분+)

## 📊 구현 상태

| Step | 작업 | 상태 | 소요시간 |
|------|------|------|---------|
| 1 | GetNextMoveDirection 구현 | ✅ 완료 | 10분 |
| 2 | UpdateMovement 구현 | ✅ 완료 | 20분 |
| 3 | ChaseTarget 구현 | ⏸️ 보류 | - |
| 4 | ValidateAndUpdatePath 구현 | ✅ 완료 | 10분 |
| 5 | Professor 클래스 연동 | ✅ 완료 | 15분 |
| 6 | 테스트 및 디버깅 | 🚧 진행 중 | 30분+ |

**총 소요 시간**: ~55분 (Step 1-5 완료) / 예상 ~90분 (전체)

---

## 🎮 프레젠테이션 시 보여줄 것

1. **"맵 기반 경로 탐색 시스템"** ✅
   - 플레이어가 이동하면 NPC가 경로를 찾아 도망
   - 맵의 복잡한 구조(벽, 복도)를 피해서 이동

2. **"A* 알고리즘 적용"** ✅
   - NavMesh 그리드 상에서 최적 경로 계산
   - 시각적으로 경로가 효율적임을 보여줌

3. **"3개 층 실시간 경로 탐색"** 🚧
   - Floor1, 2, 3 각각에서 경로 탐색이 작동
   - NPC 3명이 동시에 지능형 도망

4. **"예외 처리"** ✅
   - 경로가 없으면 NPC가 가만히 있음 (적절한 처리)

---

## 🔄 다음 단계: Scene 연동

### Floor1Scene에서 구현 필요:

// NavMesh 생성 NavMeshBuilder builder; auto navMesh = builder.Build(mapGenerator);
// PathFinder 생성 PathFinder* pathFinder = new PathFinder(navMesh.get());
// 3명의 교수님 생성 및 AIController 연동 Professor* lee = new Professor("RunLee", "RunLee"); AIController* leeAI = new AIController(pathFinder); lee->SetAIController(leeAI); lee->SetPlayerReference(player); lee->SetPatrolTarget(glm::vec3(15.0f, 0.0f, 15.0f));
// Dragon, Song 교수님도 동일하게 설정
// Scene에 추가 AddObject(lee);

---

## ✨ 완료 항목 요약

### ✅ 완료됨
- NavMesh 시스템 전체
- PathFinder (A* 알고리즘)
- AIController (모든 메서드)
- Professor 클래스 연동
- 인코딩 문제 (UTF-8)

### 🚧 진행 중
- Scene에서 3명 교수님 생성 및 초기화
- 실제 게임 환경에서 테스트

### ⏸️ 보류
- ChaseTarget (나중에 필요시 활용)

---

## 💡 주의사항

1. **PathFinder/NavMesh**: Scene에서 생성해야 함
2. **AIController**: Professor마다 독립적인 인스턴스 필요
3. **patrolTarget**: 각 교수님의 도망칠 방향 설정 필요
4. **Player 참조**: Professor가 Player를 추적할 수 있도록 설정

---

이 체크리스트를 따르면 **AI 시스템 구현의 90% 완료** 상태입니다! 🚀
마지막은 **실제 Scene에서 테스트**만 남았습니다.
