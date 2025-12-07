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

### Phase 1: AIController 핵심 기능 (필수)

#### 1-1. UpdateMovement 메서드 구현 ⭐⭐⭐ (가장 중요)

목표: NPC가 경로를 따라 이동하도록 구현

- [X] Waypoint 도달 판정
  - 현재 waypoint와 NPC 위치 거리 계산
  - 도달 거리 임계값 설정 (예: 0.5m)
  - 다음 waypoint로 진행

- [X] 경로 완료 판정
  - 마지막 waypoint 도달 시 완료 처리
  - hasReachedTarget = true 또는 IDLE로 전환

- [X] 실제 이동 적용
  - GetNextMoveDirection()으로 이동 방향 구하기
  - PROFESSOR_MOVE_SPEED (5.5 m/s) 적용
  - NPC 위치 업데이트

#### 1-2. GetNextMoveDirection 메서드 구현 ⭐⭐⭐ (가장 중요)

목표: 현재→다음 waypoint로 향하는 방향 벡터 반환

- [X] 현재 waypoint 조회
  - Path.GetNextWaypoint() 사용
  - 경로가 없으면 vec3(0,0,0) 반환

- [X] 방향 벡터 계산
  - 다음 waypoint - 현재 위치
  - glm::normalize() 정규화

- [X] 속도 벡터 변환
  - 방향 × PROFESSOR_MOVE_SPEED
  - 또는 방향만 반환해서 외부에서 속도 곱하기 (권장)

#### 1-3. ChaseTarget 메서드 구현 ⭐⭐ (2번째 중요)

목표: 플레이어 위치를 목표로 설정하고 추격

- [X] 거리 계산
  - NPC 위치 → 플레이어 위치 거리
  - glm::distance() 사용

- [X] 감지 범위 체크
  - 거리 ≤ PROFESSOR_DETECTION_RANGE (15m)
    → 목표 설정, CHASING 모드로 전환
  - 거리 > maxChaseDistance
    → 추격 포기, IDLE로 전환

- [X] 경로 계산
  - PathFinder::FindPath(현재위치, 플레이어위치) 호출
  - 경로 유효하면 currentPath 업데이트
  - 경로 없으면 STUCK 상태 (그냥 가만히 있음)

#### 1-4. ValidateAndUpdatePath 메서드 구현 ⭐ (부가)

목표: 경로가 여전히 유효한지 확인 (게임 중 맵 변경 대비)

- [X] 경로 유효성 확인
  - PathFinder::IsPathValid(currentPath) 호출

- [X] 경로 무효 시 재계산
  - 현재 위치와 목표 위치로 새 경로 계산
  - 새 경로가 없으면 STUCK 상태 유지

- [X] 호출 시기
  - 주기적 호출 (0.5초~1초마다) 추천
  - UpdateMovement 내부에 통합 가능

#### 1-5. HasReachedTarget 메서드 구현 (간단)

- [X] Path 완료 여부 확인
  - currentPath.IsComplete() 반환

---

### Phase 2: Professor 클래스 연동 ⭐⭐

#### 2-1. Professor에서 AIController 사용
// Professor.h/cpp 에서
•	[ ] AIController 멤버 변수 추가
•	[ ] Professor::Update()에서 AIController 업데이트
•	SetTargetPosition(playerPos) 호출
•	UpdateMovement(deltaTime) 호출
•	GetNextMoveDirection()으로 이동 벡터 획득
•	[ ] 이동 적용
•	Position += GetNextMoveDirection() * deltaTime
•	또는: Position += GetNextMoveDirection()


#### 2-2. NPC 3명 각각 AIController 할당

- [ ] Lee 교수님: AIController 인스턴스 1
- [ ] Dragon 교수님: AIController 인스턴스 2
- [ ] Song 교수님: AIController 인스턴스 3
- [ ] 각 AIController는 동일한 NavMesh/PathFinder 공유

---

### Phase 3: 기본 테스트 및 검증 ⭐⭐

#### 3-1. 단일 NPC 경로 추적 테스트

- [ ] Floor1에서 NPC가 플레이어를 따라가는지 확인
  - 경로 이동이 자연스러운지
  - waypoint 도달이 정확한지
  - 목표 도달 시 멈추는지

#### 3-2. 경로 없음 테스트

- [ ] 막힌 공간에서 NPC가 가만히 있는지 확인
- [ ] STUCK 상태에서 움직이지 않는지 확인

#### 3-3. 다중 NPC 테스트

- [ ] 3명의 교수님이 동시에 추격하는지 확인
- [ ] 경로 계산 중복 최소화 확인 (FPS 영향 없는지)

#### 3-4. 맵별 테스트

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