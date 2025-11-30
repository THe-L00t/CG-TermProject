# XMESH — Full Spec v2.2 (Animation‑Ready, GPU‑Optimized)

**문서 버전:** 2.2
**최종 수정:** 2025-11-28
**변경 이력:** IndexStreamHeader 구조 수정 (16→32 bytes), 실제 파일 구조 반영

---

이 문서는 `.xmesh` 포맷의 **완전한 재설계** 문서다. 이번 버전은 애니메이션(스켈레탈/스킨드 메시), 고성능 스트리밍, 런타임 압축 해제, GPU‑바로 업로드(Direct GPU Blocks), 그리고 실전에서 요구되는 모든 최적화 기법을 포함한다.

## 목표

* 런타임 로드/언팩 비용 최소화(파일 → GPU: `glBufferData` 1회 지향)
* 애니메이션 스트리밍 (on‑demand per‑track) 지원
* 파일 I/O/디스크 읽기 최소화 (mmap 친화적 레이아웃)
* 압축과 정밀도 조정을 통해 용량 최소화 + 오류 제어
* 런타임에서 CPU 부담을 줄이고 GPU에서 스키닝/애니메이션 연산 가능

---

## 개요 요약

* 포맷: Chunked Binary + Chunk별 압축(압축 알고리즘 선택 가능)
* 정점: Multi‑Stream (position, normal, tangent, uv, color, weights, boneIndices 등)
* 인덱스: 16/32bit 선택, meshoptimizer로 reindex 가능
* 스켈레톤: bone table (name,parent,bind,invBind) + optional name->index map
* 애니메이션: clip 단위, 트랙별 저장, 키프레임 압축(quantize, delta, variable sampling)
* 애니메이션 스트리밍: track table → 필요한 bone track만 디코딩
* 런타임 업로드: stream별 VBO 생성, 스킨 데이터(보통 boneMatrices)를 UBO/SSBO/TextureBuffer로 업로드

---

# 파일 레이아웃 (상위)

```
[xmesh header] (52 bytes actual data + 12 bytes padding = 64 bytes aligned)
[chunk table]  (chunk_count entries, 36 bytes each)
------------------------------------
[chunk payloads ...]
EOF
```

Chunk table에는 각 chunk의 타입, 오프셋, 압축 여부/알고리즘, 압축 전/후 크기, flags가 있고, 이를 통해 mmap, 부분 로드, skip 등을 빠르게 결정할 수 있다.

---

# 고정 헤더 (52 bytes + 12 bytes padding)

```c
struct XMeshHeader {
    char     magic[6];        // "XMESH\x1a" (6 bytes)
    uint16_t version;         // 2 (animation-enabled)
    uint32_t chunk_count;     // 청크 개수
    uint64_t chunk_table_offset; // 파일 시작부터의 오프셋 (일반적으로 52)
    uint32_t flags;           // endian, quantization defaults
    uint32_t reserved;        // 예약됨 (0으로 설정)
    uint64_t file_size;       // 전체 파일 크기 (bytes)
    // uint8_t uuid[16];      // [Optional] asset id (현재 미사용)
    // 12 bytes padding to align to 64 bytes boundary
};
// 실제 데이터: 52 bytes
// 권장 패딩: +12 bytes → 64 bytes aligned
```

**중요 변경사항 (v2.1):**
- 헤더의 실제 크기는 **52 bytes**이며, 64 bytes 정렬을 위해 12 bytes 패딩 권장
- `chunk_table_offset`은 일반적으로 **52** (헤더 직후)
- UUID 필드는 선택사항으로 변경 (확장성 고려)

---

# ChunkEntry (36 bytes)

```c
struct ChunkEntry {
    uint32_t chunk_type;         // 청크 타입 (아래 Chunk 타입 참조)
    uint64_t chunk_offset;       // 파일 시작부터의 오프셋
    uint64_t compressed_size;    // 압축된 크기 (압축 없으면 decompressed_size와 동일)
    uint64_t decompressed_size;  // 압축 해제 후 크기
    uint32_t compression;        // 압축 타입 (0=none,1=LZ4,2=ZSTD,3=MESHOPT,4=ZSTD+MESHOPT)
    uint32_t flags;              // 청크별 플래그
};
// 총 크기: 36 bytes
```

**중요 변경사항 (v2.1):**
- ChunkEntry 크기를 **36 bytes**로 명확히 정의
- 이전 문서의 40~44 bytes 불일치 해소

---

# Chunk 타입 (핵심)

```c
#define CHUNK_META            0x01  // UTF‑8 JSON metadata
#define CHUNK_VERTEX_STREAM   0x02  // vertex stream (stream header + raw/quantized bytes)
#define CHUNK_INDEX_STREAM    0x03  // index buffer (IndexStreamHeader + indices)
#define CHUNK_MESH_SECTIONS   0x04  // submesh table
#define CHUNK_SKELETON        0x05  // bone table (bind pose + invBind)
#define CHUNK_SKINNING        0x06  // skinning attributes (if not in vertex streams)
#define CHUNK_ANIMATION_INDEX 0x07  // animation clip table (headers, offsets)
#define CHUNK_ANIM_TRACK      0x08  // animation track payload (per bone, compressed)
#define CHUNK_MATERIALS       0x09  // material param table (texture references: string ids)
#define CHUNK_BOUNDING        0x0A  // bounds per LOD/section
#define CHUNK_LOD             0x0B  // LOD descriptors + remap tables
#define CHUNK_CUSTOM          0xFF  // extensibility
```

---

# Vertex Streams — GPU‑Ready Multi‑Stream

파일 단계에서 정점은 **stream 단위**로 저장된다. 각 stream은 파일에서 바로 `glBufferData`에 올릴 수 있도록 정렬된다.

```c
struct VertexStreamHeader {
    uint32_t stream_id;      // 0=pos,1=normal,2=tangent,3=uv0,4=uv1,5=color,6=weights,7=bones
    uint32_t element_size;   // bytes per element
    uint32_t count;          // element count
    uint32_t stride;         // stride if not tightly packed (0 = tightly packed)
    uint32_t format_flags;   // quantization / packing info
    uint32_t reserved;       // 예약됨
};
// 총 크기: 24 bytes
// 이후 payload: count * element_size (또는 count * stride if stride > 0)
```

**Stream ID 권장 매핑:**
- 0: Position (int16x3 quantized 또는 fp16x3)
- 1: Normal (int16x3 quantized 또는 octahedral encoding)
- 2: Tangent (int16x4 또는 octahedral + sign)
- 3: UV0 (fp16x2)
- 4: UV1 (fp16x2, optional)
- 5: Color (u8x4 RGBA)
- 6: Bone Weights (u8x4 normalized 또는 fp16x4)
- 7: Bone Indices (u8x4 또는 u16x4)

**권장 저장 포맷(예):**
* positions: center+scale quantized (int16x3) 또는 fp16x3
* normals/tangents: octahedral 또는 10|10|10|2 packing
* uvs: fp16x2
* weights: u8[4] normalized (or fp16)
* bone indices: u8[4]

**장점**: 필요한 stream만 로드하여 메모리 사용 최적화 가능.

---

# Index Streams — Optimized Index Buffer

```c
struct IndexStreamHeader {
    uint32_t header_size;      // 헤더 크기 (32 bytes, 버전/검증용)
    uint32_t index_count;      // 총 인덱스 개수
    uint32_t index_type;       // 0=uint32, 1=uint16
    uint32_t primitive_type;   // 0=triangles, 1=lines, 2=points
    uint32_t reserved[4];      // 예약됨 (확장성)
};
// 총 크기: 32 bytes
// 이후 payload: index_count * (index_type ? 2 : 4) bytes
```

**중요 변경사항 (v2.2):**
- ⚠️ **헤더 크기가 16 bytes → 32 bytes로 변경됨** (실제 구현 기준)
- 첫 번째 필드는 `header_size` (32 고정) - 버전 검증 및 확장성
- `index_type`: 0=uint32 (4 bytes), 1=uint16 (2 bytes)
- 예: 1,418,724개 인덱스 × 4 bytes = 5,674,896 bytes payload

**파싱 예제:**
```cpp
IndexStreamHeader header;
fread(&header, sizeof(header), 1, file);

if (header.header_size != 32) {
    error("Invalid INDEX_STREAM header version");
}

size_t bytes_per_index = (header.index_type == 0) ? 4 : 2;
std::vector<uint8_t> indices(header.index_count * bytes_per_index);
fread(indices.data(), bytes_per_index, header.index_count, file);
```

**최적화 팁:**
- 정점 수가 65,536 미만이면 uint16 사용 권장
- meshoptimizer의 `meshopt_optimizeVertexCache`로 cache-friendly 순서 재배치
- meshoptimizer 압축 적용 시 인덱스 데이터는 2~4배 압축 가능

---

# Skeleton (CHUNK_SKELETON)

Skeleton chunk에는 bone 리스트와 bind pose, inverse bind pose를 저장한다.

```c
struct SkeletonHeader {
    uint32_t bone_count;         // 본 개수
    uint32_t name_table_offset;  // 이름 테이블 오프셋 (청크 시작부터), 0이면 없음
};
// 총 크기: 8 bytes

struct BoneEntry {
    uint32_t name_offset;      // name table 내 오프셋, -1이면 이름 없음
    int32_t  parent_index;     // 부모 본 인덱스, -1이면 루트
    float    local_bind[16];   // column-major mat4 (bind pose local transform)
    float    inv_bind[16];     // inverse bind matrix (model space → bone space)
    uint32_t flags;            // 본별 플래그
};
// 총 크기: 4 + 4 + 64 + 64 + 4 = 140 bytes per bone

// 이후: bone_count * 140 bytes
// 그 다음: [name table - contiguous null-terminated UTF8 strings]
```

**Name Table 구조:**
- 연속된 null-terminated UTF-8 문자열들
- 각 BoneEntry의 `name_offset`은 name table 시작부터의 오프셋
- 예: "Root\0Spine\0Head\0LeftArm\0..."

**검증 실례 (RunLee.xmesh):**
- 52개 본
- Name table offset: 7,504 bytes
- Bone data: 52 × 140 = 7,280 bytes
- Name table 시작: 8 + 7,280 = 7,288 bytes ✓

---

# Skinning Strategy (Runtime)

* Vertex stores bone indices & weights (u8/u16 combos) as a stream or within interleaved VBOs.
* Runtime will build bone matrix palette: `mat4 palette[numBonesUsed]`
* Upload options:
  * UBO (fast, limited size per draw ~128 matrices) — good for most characters
  * SSBO / Texture Buffer for many bones / GPU skinning with compute

권장: 기본 UBO(128 mats) + fallback SSBO.

---

# Animation Storage (CHUNK_ANIMATION_INDEX + CHUNK_ANIM_TRACK)

애니메이션은 **클립 인덱스 테이블**(CHUNK_ANIMATION_INDEX)과 **개별 트랙(chunk per track)** (CHUNK_ANIM_TRACK)으로 분리된다.

## Animation Index (clip table)

```c
struct AnimIndexHeader {
    uint32_t clip_count;  // 애니메이션 클립 개수
};
// 총 크기: 4 bytes

struct AnimClipEntry {
    uint32_t name_offset;        // 클립 이름 오프셋 (청크 내)
    float    duration;           // 애니메이션 길이 (초)
    float    sample_rate;        // 샘플링 주파수 (fps)
    uint32_t num_bone_tracks;    // 이 클립의 본 트랙 개수
    uint32_t track_table_offset; // 트랙 테이블 오프셋 (이 엔트리부터)
};
// 총 크기: 20 bytes per clip

struct TrackTableEntry {
    uint32_t bone_index;         // 어떤 본의 트랙인지
    uint32_t num_keys;           // 키프레임 개수
    uint64_t track_chunk_offset; // 해당 ANIM_TRACK 청크의 파일 오프셋
};
// 총 크기: 16 bytes per track
```

**주의사항 (v2.1):**
- `duration`과 `sample_rate`는 float이므로 정확히 4 bytes씩
- 파일에서 읽을 때 반드시 올바른 endianness로 언팩할 것
- 0이 나온다면 파싱 오류를 의심

## Anim Track Chunk (per‑bone)

* 트랙은 **bone 단위**로 완전히 분리되어 저장된다 — 런타임에서 필요한 bone만 디코딩 가능

```c
struct AnimTrackHeader {
    uint32_t bone_index;      // 본 인덱스
    uint32_t num_keys;        // 키프레임 개수
    uint8_t  key_format;      // 비트플래그: POS_COMPRESSED, ROT_COMPRESSED, SCL_COMPRESSED
    uint8_t  time_format;     // 시간 저장 포맷 (0=uint16 normalized, 1=float)
    uint16_t reserved;        // 예약됨
};
// 총 크기: 12 bytes
// 이후 payload: 키프레임 데이터 (포맷에 따라 다름)
```

**Key Format Flags:**
```c
#define KEY_FORMAT_POS_QUANTIZED   0x01  // Position quantized to int16
#define KEY_FORMAT_ROT_COMPRESSED  0x02  // Rotation compressed (quaternion 48-bit)
#define KEY_FORMAT_SCL_QUANTIZED   0x04  // Scale quantized to int16
#define KEY_FORMAT_UNIFORM_TIME    0x08  // Uniform time sampling (implicit time)
#define KEY_FORMAT_DELTA_ENCODED   0x10  // Delta encoding (first key absolute, rest deltas)
```

**검증 실례 (RunLee.xmesh):**
- Bone 1: 19 keys, format 0x2A
- Bone 9: 9 keys, format 0x2A
- Bone 10: 9 keys, format 0x2A
- Bone 29: 11 keys, format 0x2A

### Key Payload Strategies (권장)

* **Packed keys**: for each key store quantized pos/rot/scale without padding.
* **Delta + Predictor**: store first key as base, subsequent keys as small deltas (int16 or int8) — good for smooth motion.
* **Uniform sampling table**: if clip is sampled uniformly, store only sampled values and use index arithmetic for time → sample lookup.

**파일 저장 예시 (most efficient)**

* If clip is uniformly sampled: store a compact array (N samples) of packed vectors (pos3,rot4,scale3), with time implicit.
* If non-uniform: store per-key time (uint16 normalized to [0,duration]) + packed transforms.

---

# Keyframe Compression / Quantization Recommendations

* Position: origin + scale quantization → int16 or int24 per component (bounded error)
* Rotation: compress quaternion to 48 bits (3 components + sign) or 32-bit small quaternion compression
* Scale: int16 per component
* Time: uint16 normalized relative to clip duration (max 65535 samples)
* Use error threshold pruning: remove keys whose interpolation error < eps

압축률 목표: raw FBX 애니메이션 대비 4x~20x 감소 (클립 성격에 따라 다름)

---

# Animation Streaming (On‑Demand)

* CHUNK_ANIM_TRACK chunk는 독립된 청크로 배치되어 있어 `chunk_table`에서 오프셋만 알면 mmap 포인터로 직접 접근 가능.
* 런타임은 필요 트랙만 읽고(혹은 압축해제 후) GPU로 업로드.
* Active bones list(예: torso only)로 필요한 tracks만 로드하면 메모리/IO 절약이 큼.

**Streaming Flow:**

1. Read AnimIndex → decide clip
2. For clip, read track table → determine which bone tracks required
3. For each required track: mmap & decompress track payload into decode buffer
4. Interpolate per-frame and make bone matrices
5. Upload bone matrices to UBO/SSBO

---

# Compression Strategy (강력 권장)

**v2.1 추가: 압축 가이드라인**

파일 크기를 5~8배 줄이기 위해 다음 압축 전략을 권장:

| 청크 타입 | 권장 압축 | 예상 압축률 | 이유 |
|----------|----------|------------|------|
| VERTEX_STREAM | MESHOPT + ZSTD | 4~6x | Topology-aware, 높은 중복성 |
| INDEX_STREAM | MESHOPT + ZSTD | 5~8x | 인덱스 패턴 최적화 |
| SKELETON | ZSTD | 2~3x | 텍스트(이름) 압축 효과 |
| ANIM_TRACK | ZSTD | 3~5x | 키프레임 델타 압축과 시너지 |
| MESH_SECTIONS | NONE | - | 너무 작음 (수십 bytes) |

**압축 알고리즘 선택:**
```c
enum CompressionType {
    COMPRESSION_NONE       = 0,  // 압축 없음
    COMPRESSION_LZ4        = 1,  // 빠른 압축/해제 (2~3x)
    COMPRESSION_ZSTD       = 2,  // 높은 압축률 (3~5x), 적당한 속도
    COMPRESSION_MESHOPT    = 3,  // meshoptimizer 전용 (4~6x)
    COMPRESSION_ZSTD_MESHOPT = 4 // meshopt + ZSTD 연쇄 (5~8x, 최고 압축)
};
```

**실제 압축 효과 예시 (RunLee.xmesh 기준):**
- 압축 전: 9.4 MB
- LZ4 적용: ~3~4 MB (2~3x)
- ZSTD 적용: ~2~3 MB (3~5x)
- MESHOPT+ZSTD: ~1~2 MB (5~8x) ✅ 권장

---

# Multiple Clips / Blending / Runtime API

* Clip descriptor includes wrap mode, in/out blend time, root motion flags
* Runtime should support per-clip weight blending and crossfade
* For GPU skinning, blending should compute final bone matrices on CPU or GPU (compute shader) depending on performance profile

---

# On‑Disk Example: Small Character Asset

```
Header (52 bytes + 12 padding)
ChunkTable: [
  skeleton,
  v_stream_pos, v_stream_nml, v_stream_uv,
  idx,
  sections,
  skinning,
  anim_index,
  anim_track_0, anim_track_1, ...,
  materials
]
Payloads (각 청크는 독립적으로 압축 가능)
```

**실제 사례 (RunLee.xmesh):**
- Skeleton: 52 bones (8.5 KB)
- Vertex Streams: 3 streams, 262K vertices (4.15 MB)
- Index Stream: 5.41 MB
- Animation: 1 clip, 4 bone tracks (1.1 KB)
- Total: 9.4 MB (압축 없음) → 압축 적용 시 1~2 MB 예상

---

# Conversion Pipeline (fbx2xmesh with Animation Extraction)

1. Load FBX with Assimp (triangulate, gen normals, calc tangents)
2. Deduplicate vertices (pos+nml+uv+skin) -> build stream buffers
3. Quantize streams (according to thresholds)
4. Build indices (16-bit if possible)
5. Export skeleton: compute bind pose & inverse bind
6. Export skinning streams: bone indices, weights
7. For each animation clip:
   * sample or extract keyframes per bone
   * perform key reduction (prune small deltas)
   * quantize keys (pos->int16, rot->packed48, time->uint16)
   * write track chunk per bone (optionally uniform sample table)
8. Optionally: meshopt compress position/index topology
9. Per‑chunk compress with LZ4/ZSTD
10. Write header + chunk table + payloads

---

# Runtime Loader API (Suggested)

```cpp
struct XMeshAsset { /* header + chunk index ptrs */ };

// 파일 로드 (mmap 권장)
XMeshAsset* load_xmesh_mmap(const char* path);

// GPU 업로드
void upload_mesh_to_gpu(XMeshAsset* a, GLcontext ctx); // creates VBO/EBO per stream

// Animation
AnimClipHandle get_clip(XMeshAsset*, name_or_index);
void stream_and_decode_tracks(
    XMeshAsset*,
    AnimClipHandle,
    const std::vector<int>& bones_needed,
    DecodeBuffer& out
);
void compute_and_upload_palette(DecodeBuffer&, GLuint ubo_or_ssbo);

// 압축 해제
void decompress_chunk(
    const void* compressed_data,
    size_t compressed_size,
    CompressionType type,
    void* output_buffer,
    size_t decompressed_size
);
```

---

# GLSL Skinning Example (UBO Approach)

```glsl
#version 450 core

layout(std140, binding = 0) uniform Bones {
    mat4 u_Bones[128];
};

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in uvec4 in_boneIndices;
layout(location = 3) in vec4 in_boneWeights;

out vec3 v_normal;

uniform mat4 u_MVP;

void main() {
    // Skinning
    mat4 skinMat =
        u_Bones[in_boneIndices.x] * in_boneWeights.x +
        u_Bones[in_boneIndices.y] * in_boneWeights.y +
        u_Bones[in_boneIndices.z] * in_boneWeights.z +
        u_Bones[in_boneIndices.w] * in_boneWeights.w;

    vec4 skinned_pos = skinMat * vec4(in_position, 1.0);
    vec4 skinned_nml = skinMat * vec4(in_normal, 0.0);

    gl_Position = u_MVP * skinned_pos;
    v_normal = normalize(skinned_nml.xyz);
}
```

---

# Design Tradeoffs & Notes

* **Per‑track chunk**: +streaming, +partial load, +caching individual bones; -more chunk table entries
* **Uniform sample tables**: extremely fast to access but may waste space for sparse keyframes
* **UBO vs SSBO vs TextureBuffer**: UBO fastest for small palettes; SSBO/TextureBuffer scalable
* **Quantization errors**: choose tolerances per-asset; conversion tool should expose parameters
* **압축 트레이드오프**: ZSTD+MESHOPT는 최고 압축률이지만 해제 시간 증가, LZ4는 빠르지만 낮은 압축률

---

# CLI / Tooling Recommendations

```bash
# 기본 변환 (압축 없음)
fbx2xmesh --in char.fbx --out char.xmesh --anim

# 최적화 변환 (권장)
fbx2xmesh --in char.fbx --out char.xmesh \
  --anim \
  --sample-rate 30 \
  --pos-quant int16 \
  --rot-quant packed48 \
  --compress zstd+meshopt \
  --error-threshold 0.001

# 압축 레벨 제어
fbx2xmesh --in char.fbx --out char.xmesh \
  --compress-level fast    # LZ4
  --compress-level medium  # ZSTD default
  --compress-level max     # ZSTD max + MESHOPT
```

---

# Validation & Debugging Tools

```bash
# 파일 검증
xmesh-validate char.xmesh

# 상세 분석
xmesh-analyze char.xmesh --verbose

# 청크별 압축 효과 보기
xmesh-analyze char.xmesh --compression-report

# 특정 청크 추출
xmesh-extract char.xmesh --chunk 7 --output anim_track.bin
```

---

# Version History

## v2.2 (2025-11-28) 🔥 Critical Fix
- ✅ **IndexStreamHeader 구조 수정: 16 bytes → 32 bytes**
- ✅ 첫 번째 필드를 `header_size`로 변경 (버전 검증용)
- ✅ `index_type` 필드 추가 (0=uint32, 1=uint16)
- ✅ RunLee.xmesh 실제 파일 구조와 100% 일치
- ⚠️ **Breaking Change**: 기존 16-byte 헤더 로더는 수정 필요

## v2.1 (2025-11-28)
- ✅ 헤더 크기를 52 bytes로 명확화 (+ 12 bytes padding 권장)
- ✅ ChunkEntry 크기를 36 bytes로 명확화
- ✅ 압축 가이드라인 추가 (MESHOPT + ZSTD 권장)
- ✅ 실제 구현 사례(RunLee.xmesh) 기반 검증 및 명세 개선
- ✅ AnimClipEntry 필드 순서 및 타입 명확화

## v2.0 (이전)
- Animation support
- Per-bone track streaming
- Multi-stream vertex format
- Chunk-based compression

---

# Next Deliverables

* `fbx2xmesh` full implementation: animation sampling, key reduction, per-track chunks, quantization + chunk compression
* `xmesh_loader` runtime: mmap, per-track streaming, LZ4/ZSTD decompress, GL uploader, UBO/SSBO support
* Animation player: CPU blending, crossfade, GPU skinning compute shader
* Unit tests + sample assets
* Performance benchmarks (load time, memory usage, rendering fps)

---

# References & Standards

- **Endianness**: Little-endian (Intel/AMD x86-64 standard)
- **Float Format**: IEEE 754 single precision (32-bit)
- **Matrix Format**: Column-major (OpenGL standard)
- **Quaternion Format**: (x, y, z, w)
- **UV Origin**: Bottom-left (OpenGL standard)

---

**문서 작성자:** 이태형
**다음 리뷰:** 2025-12-01
