# XMesh Format v2 - 사용 가이드

## 개요
XMesh는 GPU 최적화된 메시 포맷으로, 스켈레톤 애니메이션과 스트리밍을 지원합니다.

## 포맷 구조

### 파일 레이아웃
```
[XMeshHeader] (64 bytes)
[ChunkTable] (chunk_count entries)
-----------------------------------
[Chunk Payloads...]
EOF
```

### 청크 타입
- `CHUNK_META` (0x01): JSON 메타데이터
- `CHUNK_VERTEX_STREAM` (0x02): 정점 스트림
- `CHUNK_INDEX_STREAM` (0x03): 인덱스 버퍼
- `CHUNK_MESH_SECTIONS` (0x04): 서브메시 테이블
- `CHUNK_SKELETON` (0x05): 본 테이블
- `CHUNK_SKINNING` (0x06): 스키닝 속성
- `CHUNK_ANIMATION_INDEX` (0x07): 애니메이션 클립 테이블
- `CHUNK_ANIM_TRACK` (0x08): 애니메이션 트랙
- `CHUNK_MATERIALS` (0x09): 재질 파라미터
- `CHUNK_BOUNDING` (0x0A): 바운딩 박스
- `CHUNK_LOD` (0x0B): LOD 정보

## C++ 사용법

### 1. 메시 로드
```cpp
ResourceManager* resMgr = new ResourceManager();
resMgr->LoadXMesh("character", "models/character.xmesh");
```

### 2. 애니메이션 플레이어 설정
```cpp
AnimationPlayer animPlayer;
const XMeshData* meshData = resMgr->GetXMeshData("character");
animPlayer.SetMesh(meshData);

// 애니메이션 재생
animPlayer.PlayAnimation("walk", true); // 이름으로
// 또는
animPlayer.PlayAnimation(0, true); // 인덱스로
```

### 3. 게임 루프
```cpp
void Update(float deltaTime) {
    animPlayer.Update(deltaTime);
}

void Render() {
    glm::mat4 model = glm::mat4(1.0f);

    // 스켈레톤 애니메이션 렌더링
    renderer->RenderAnimatedMesh(
        "character",
        animPlayer.GetFinalTransforms(),
        model
    );
}
```

### 4. 애니메이션 제어
```cpp
// 재생 제어
animPlayer.Pause();
animPlayer.Resume();
animPlayer.Stop();

// 재생 속도 변경
animPlayer.SetPlaybackSpeed(2.0f); // 2배속

// 본 인덱스 조회
int headBoneIndex = animPlayer.GetBoneIndex("Head");
```

## GLSL 셰이더 예제

### Vertex Shader (스키닝 지원)
```glsl
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 6) in vec4 aBoneWeights;   // stream 6
layout(location = 7) in uvec4 aBoneIndices;  // stream 7

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// 본 팔레트 (최대 128개)
layout(std140) uniform Bones {
    mat4 uBones[128];
};

uniform int uBoneCount;

out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    // 스키닝 계산
    mat4 skinMatrix = mat4(0.0);
    if (uBoneCount > 0) {
        skinMatrix =
            uBones[aBoneIndices.x] * aBoneWeights.x +
            uBones[aBoneIndices.y] * aBoneWeights.y +
            uBones[aBoneIndices.z] * aBoneWeights.z +
            uBones[aBoneIndices.w] * aBoneWeights.w;
    } else {
        skinMatrix = mat4(1.0);
    }

    vec4 skinnedPosition = skinMatrix * vec4(aPosition, 1.0);
    vec4 skinnedNormal = skinMatrix * vec4(aNormal, 0.0);

    gl_Position = uProjection * uView * uModel * skinnedPosition;
    vNormal = normalize((uModel * skinnedNormal).xyz);
    vTexCoord = aTexCoord;
}
```

### Fragment Shader
```glsl
#version 330 core

in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uColor;
uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

out vec4 FragColor;

void main() {
    // 간단한 Phong 라이팅
    vec3 ambient = 0.3 * uLightColor;

    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightPos - gl_FragCoord.xyz);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    vec3 result = (ambient + diffuse) * uColor;
    FragColor = vec4(result, 1.0);
}
```

## 스트림 ID 매핑

| Stream ID | 속성 | 포맷 예시 |
|-----------|------|-----------|
| 0 | Position | float3 or int16x3 |
| 1 | Normal | float3 or 10_10_10_2 |
| 2 | Tangent | float3 or 10_10_10_2 |
| 3 | UV0 | float2 or half2 |
| 4 | UV1 | float2 or half2 |
| 5 | Color | RGBA8 |
| 6 | BoneWeights | float4 or u8[4] normalized |
| 7 | BoneIndices | u8[4] or u16[4] |

## 압축 옵션

### Vertex Stream 압축
- **Position**: int16x3 quantization, center+scale
- **Normal**: octahedral 또는 10_10_10_2 packing
- **UV**: half2 (float16)
- **Weights**: u8[4] normalized

### Animation 압축
- **Position**: int16 per component
- **Rotation**: 48-bit packed quaternion
- **Scale**: int16 per component
- **Time**: uint16 normalized

### Chunk 압축
- None (0): 압축 없음
- LZ4 (1): 빠른 압축/해제
- ZSTD (2): 높은 압축률
- MESHOPT (3): 메시 최적화
- ZSTD+MESHOPT (4): 최대 압축

## 성능 최적화 팁

### 1. UBO vs SSBO
```cpp
// UBO (빠름, 제한적)
glBindBuffer(GL_UNIFORM_BUFFER, ubo);
glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 128,
             transforms.data(), GL_DYNAMIC_DRAW);

// SSBO (느림, 대용량)
glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * boneCount,
             transforms.data(), GL_DYNAMIC_DRAW);
```

### 2. 필요한 트랙만 로드
```cpp
// 필요한 본만 활성화
std::vector<int> activeBones = {0, 1, 2, 5, 10}; // torso only
// 해당 트랙만 디코딩하여 메모리 절약
```

### 3. LOD 시스템
```cpp
// 거리에 따라 LOD 선택
float distance = glm::distance(camera->GetPosition(), objectPosition);
size_t lodLevel = (distance < 10.0f) ? 0 : 1;
renderer->RenderXMeshSection("character", lodLevel, model);
```

## 파일 변환 (향후 지원)

```bash
# FBX를 xmesh로 변환
fbx2xmesh --in character.fbx --out character.xmesh \
          --anim --sample-rate 30 \
          --pos-quant int16 --rot-quant packed48 \
          --compress lz4
```

## 트러블슈팅

### 애니메이션이 재생되지 않음
1. 스켈레톤이 로드되었는지 확인: `mesh->has_skeleton`
2. 애니메이션 클립이 있는지 확인: `mesh->animations.size()`
3. Update() 함수가 매 프레임 호출되는지 확인

### 메시가 깨져 보임
1. 본 변환 행렬이 올바르게 전달되었는지 확인
2. 셰이더에서 스키닝 계산이 올바른지 확인
3. Bone weights가 정규화되어 있는지 확인 (합이 1.0)

### 성능 문제
1. 압축된 포맷 사용
2. LOD 시스템 활용
3. 필요한 트랙만 로드
4. GPU 스키닝 사용 (Compute Shader)

## 참고 자료

- OpenGL Skinning Tutorial: https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
- glTF 2.0 Spec: https://github.com/KhronosGroup/glTF
- Meshoptimizer: https://github.com/zeux/meshoptimizer
