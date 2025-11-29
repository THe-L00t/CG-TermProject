# XMesh 파일 로딩 및 렌더링 테스트 가이드

## 개요

이 문서는 .xmesh 파일 포맷(v2.1 명세)을 읽어오고 GPU에 업로드하여 렌더링하는 기능을 테스트하기 위한 가이드입니다.

## 주요 수정 사항

### 1. 헤더 구조체 업데이트 (ResourceManager.h)

XMesh v2.1 명세에 맞춰 다음 구조체들을 업데이트했습니다:

- **XMeshHeader**: 52 bytes (실제 데이터) + 12 bytes padding 권장
- **ChunkEntry**: 36 bytes (명확히 정의)
- **IndexStreamHeader**: 구조 재정의
  - `index_count`: 총 인덱스 개수
  - `index_size`: 2 (uint16) 또는 4 (uint32)
  - `primitive_type`: 0=triangles, 1=lines, 2=points
- **SkeletonHeader**: 8 bytes
- **BoneEntry**: 140 bytes per bone
- **AnimClipEntry**: 20 bytes per clip

### 2. LoadXMesh 함수 개선 (ResourceManager.cpp:202-498)

주요 개선사항:

#### 2.1 IndexStreamHeader 파싱 수정
```cpp
// 이전 (잘못된 방식)
meshData.index_type = (indexHeader->index_type == 16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

// 수정 (v2.1 명세에 맞춤)
meshData.index_type = (indexHeader->index_size == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
```

#### 2.2 Vertex Attribute 매핑 개선

XMesh 스트림 ID를 OpenGL vertex attribute location에 정확히 매핑:

| Stream ID | 의미 | Shader Location | Component Count | Data Type |
|-----------|------|-----------------|-----------------|-----------|
| 0 | Position | 0 | 3 | GL_FLOAT |
| 1 | Normal | 2 | 3 | GL_FLOAT |
| 3 | UV0 | 1 | 2 | GL_FLOAT |
| 6 | Bone Weights | 3 | 4 | GL_FLOAT |
| 7 | Bone Indices | 4 | 4 | GL_UNSIGNED_BYTE |

### 3. RenderXMesh 함수 (Renderer.cpp:152-226)

기존 함수는 이미 잘 구현되어 있습니다:

- VAO 바인드
- Mesh sections 처리
- `glDrawElementsBaseVertex` 사용 (섹션이 있는 경우)
- `glDrawElements` 사용 (섹션이 없는 경우)

## 테스트 방법

### 1. 빌드 및 실행

프로젝트를 빌드하고 실행하면 자동으로 다음 과정이 진행됩니다:

1. **ResourceManager 초기화** (Engine.cpp:43-45)
2. **Asset 로딩** (Engine.cpp:135-162)
   - bugatti.obj (fallback)
   - RunLee.xmesh
   - RunSong.xmesh
   - RunDragon.xmesh

### 2. 콘솔 출력 확인

정상적으로 로드되면 다음과 같은 출력이 나타납니다:

```
=== Loading Assets ===

--- Loading XMesh files ---
File loaded: [파일크기] bytes
Magic: 58 4d 45 53 48 1a
Loading XMesh 'RunLee' version 2 with [청크개수] chunks
Chunk table offset: 52
File size: [파일크기]

Processing chunk 0: type=0x2, offset=[오프셋], size=[크기]
  Stream 0: [정점개수] elements, 12 bytes each
Processing chunk 1: type=0x2, offset=[오프셋], size=[크기]
  Stream 1: [정점개수] elements, 12 bytes each
Processing chunk 2: type=0x2, offset=[오프셋], size=[크기]
  Stream 3: [정점개수] elements, 8 bytes each
Processing chunk 3: type=0x3, offset=[오프셋], size=[크기]
  Indices: [인덱스개수] (size: 4 bytes, type: 0)
Processing chunk 4: type=0x5, offset=[오프셋], size=[크기]
  Skeleton: [본개수] bones

Starting GPU upload...
Uploading stream 0 (ID=0, size=[크기], count=[개수], element_size=12)
  -> Enabled attrib 0 with 3 components
Uploading stream 1 (ID=1, size=[크기], count=[개수], element_size=12)
  -> Enabled attrib 2 with 3 components
Uploading stream 2 (ID=3, size=[크기], count=[개수], element_size=8)
  -> Enabled attrib 1 with 2 components
Uploading index buffer (size=[크기])
GPU upload complete

XMesh 'RunLee' loaded successfully with [인덱스개수] indices
SUCCESS: RunLee.xmesh loaded
```

### 3. 렌더링 확인

프로그램이 실행되면 다음 중 하나가 화면에 표시됩니다:

1. **XMesh 로딩 성공**: RunLee 모델이 화면에 렌더링됨
2. **XMesh 로딩 실패**: Fallback으로 bugatti 큐브가 렌더링됨

렌더링 로직은 Engine.cpp:96-122에서 처리됩니다.

## 파일 구조

```
Deadline_professor/
├── ResourceManager.h        # XMesh 구조체 정의
├── ResourceManager.cpp      # LoadXMesh 구현
├── Renderer.h               # 렌더링 함수 선언
├── Renderer.cpp             # RenderXMesh 구현
├── Engine.cpp               # Asset 로딩 및 렌더링 루프
├── basic.vert               # Vertex 셰이더
├── basic.frag               # Fragment 셰이더
├── RunLee.xmesh             # 테스트 모델 1
├── RunSong.xmesh            # 테스트 모델 2
└── RunDragon.xmesh          # 테스트 모델 3
```

## 주요 함수

### 1. ResourceManager::LoadXMesh()

**위치**: ResourceManager.cpp:202-498

**기능**:
- XMesh 파일을 메모리에 로드
- 헤더 검증 (magic number, version)
- 청크 테이블 파싱
- 각 청크 타입별 처리:
  - CHUNK_VERTEX_STREAM: 정점 데이터
  - CHUNK_INDEX_STREAM: 인덱스 데이터
  - CHUNK_MESH_SECTIONS: 서브메시 정보
  - CHUNK_SKELETON: 본 구조
  - CHUNK_ANIMATION_INDEX: 애니메이션 클립 정보
- GPU 업로드 (VBO, EBO 생성 및 데이터 전송)

**반환값**: `true` (성공) / `false` (실패)

### 2. Renderer::RenderXMesh()

**위치**: Renderer.cpp:152-226

**기능**:
- XMeshData를 가져와서 화면에 렌더링
- 셰이더 설정 (basic)
- 변환 행렬 설정 (model, view, projection)
- 라이팅 파라미터 설정
- 섹션별 렌더링 또는 전체 렌더링

**파라미터**:
- `meshName`: 로드된 메시 이름 (예: "RunLee")
- `modelMatrix`: 모델 변환 행렬 (기본값: 단위 행렬)

## 트러블슈팅

### 문제 1: "Invalid XMesh file: magic mismatch"

**원인**: 파일 형식이 올바르지 않거나 손상됨

**해결**:
- 파일이 올바른 XMesh v2 포맷인지 확인
- Magic number가 "XMESH\x1a" 또는 "XMESH\0"인지 확인

### 문제 2: "Warning: No vertex streams found"

**원인**: CHUNK_VERTEX_STREAM이 파일에 없음

**해결**:
- fbx2xmesh 도구로 재변환
- 파일 청크 구조 확인

### 문제 3: 모델이 화면에 안 보임

**가능한 원인**:
1. 모델 스케일이 너무 크거나 작음 → Engine.cpp:103, 112에서 스케일 조정
2. 카메라 위치가 부적절 → Engine.cpp:56-62에서 카메라 위치 조정
3. 인덱스 타입 불일치 → ResourceManager.cpp:307 확인

### 문제 4: "Invalid component count"

**원인**: Stream element_size가 예상과 다름

**해결**:
- ResourceManager.cpp:480-504의 매핑 로직 확인
- 파일의 실제 element_size 출력해서 확인

## 성능 고려사항

- **파일 로딩**: 전체 파일을 메모리에 로드하므로 큰 파일은 로딩 시간이 길 수 있음
- **압축**: 현재 버전은 압축을 지원하지 않음 (v2.1 명세에는 LZ4, ZSTD, MESHOPT 압축 옵션 있음)
- **GPU 업로드**: GL_STATIC_DRAW 사용, 한 번 업로드 후 재사용

## 향후 개선사항

1. **압축 지원**: ZSTD, LZ4, MESHOPT 압축 해제
2. **스트리밍**: 큰 파일을 부분적으로 로드
3. **애니메이션 재생**: AnimationPlayer 통합
4. **LOD 지원**: CHUNK_LOD 처리
5. **Material 시스템**: CHUNK_MATERIALS 활용

## 참고 문서

- XMesh v2.1 명세서 (프로젝트 루트)
- OpenGL Vertex Specification: https://www.khronos.org/opengl/wiki/Vertex_Specification
- glDrawElementsBaseVertex: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsBaseVertex.xhtml
