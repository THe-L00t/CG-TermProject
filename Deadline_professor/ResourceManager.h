#pragma once
#include "TotalHeader.h"
#include <gl/glm/gtc/quaternion.hpp>

struct Vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
};

struct ObjData {
	std::string name;
	GLuint VBO{};
	GLuint EBO{};
	size_t indexCount{};
};

// XMesh 파일 포맷 구조체 (Version 2.1)
#pragma pack(push, 1)
struct XMeshHeader {
	char magic[6];            // "XMESH\x1a" (6 bytes)
	uint16_t version;         // 버전 2 (animation-enabled)
	uint32_t chunk_count;     // 청크 개수
	uint64_t chunk_table_offset; // 청크 테이블 오프셋 (일반적으로 52)
	uint32_t flags;           // 플래그 (엔디안, quantization 기본값)
	uint32_t reserved;        // 예약
	uint64_t file_size;       // 파일 크기
	// 실제 데이터: 52 bytes
	// 권장 패딩: +12 bytes → 64 bytes aligned (파일에서 자동 처리)
};

struct ChunkEntry {
	uint32_t chunk_type;      // 청크 타입
	uint64_t chunk_offset;    // 파일 시작부터의 오프셋
	uint64_t compressed_size; // 압축된 크기
	uint64_t decompressed_size; // 압축 해제된 크기
	uint32_t compression;     // 0=none, 1=LZ4, 2=ZSTD, 3=MESHOPT, 4=ZSTD+MESHOPT
	uint32_t flags;           // 청크별 플래그
	// 총 크기: 36 bytes
};

struct VertexStreamHeader {
	uint32_t stream_id;      // 0=pos, 1=normal, 2=tangent, 3=uv0, 4=uv1, 5=color, 6=weights, 7=bones
	uint32_t element_size;   // 요소당 바이트 크기
	uint32_t count;          // 요소 개수
	uint32_t stride;         // stride (0이면 tightly packed)
	uint32_t format_flags;   // quantization / packing
	uint32_t reserved;
};

struct IndexStreamHeader {
	uint32_t header_size;      // 헤더 크기 (32 bytes, 버전/검증용)
	uint32_t index_count;      // 총 인덱스 개수
	uint32_t index_type;       // 0=uint32, 1=uint16
	uint32_t primitive_type;   // 0=triangles, 1=lines, 2=points
	uint32_t reserved[4];      // 예약됨 (확장성)
	// 총 크기: 32 bytes
};

struct MeshSection {
	uint32_t material_id;
	uint32_t index_start;
	uint32_t index_count;
	uint32_t vertex_start;
	uint32_t vertex_count;
	uint16_t lod_level;
	uint16_t flags;
};

// 스켈레톤 구조체 (v2.1 포맷)
struct SkeletonHeader {
	uint32_t bone_count;         // 본 개수
	uint32_t name_table_offset;  // 이름 테이블 오프셋 (청크 시작부터), 0이면 없음
	// 총 크기: 8 bytes
};

struct BoneEntry {
	uint32_t name_offset;      // 이름 테이블 내 오프셋, -1이면 없음
	int32_t parent_index;      // 부모 본 인덱스 (-1이면 루트)
	float local_bind[16];      // column-major mat4 (bind pose local)
	float inv_bind[16];        // inverse bind matrix
	uint32_t flags;            // 본별 플래그
	// 총 크기: 4 + 4 + 64 + 64 + 4 = 140 bytes per bone
};

// 애니메이션 인덱스 (CHUNK_ANIMATION_INDEX)
struct AnimIndexHeader {
	uint32_t clip_count;  // 애니메이션 클립 개수
	// 총 크기: 4 bytes
};

struct AnimClipEntry {
	uint32_t name_offset;        // 클립 이름 오프셋 (청크 내)
	float duration;              // 애니메이션 길이 (초)
	float sample_rate;           // 샘플링 주파수 (fps)
	uint32_t num_bone_tracks;    // 이 클립의 본 트랙 개수
	uint32_t track_table_offset; // 트랙 테이블 오프셋 (이 엔트리부터)
	// 총 크기: 20 bytes per clip
};

struct TrackTableEntry {
	uint32_t bone_index;         // 어떤 본의 트랙인지
	uint32_t num_keys;           // 키프레임 개수
	uint64_t track_chunk_offset; // 해당 ANIM_TRACK 청크의 파일 오프셋
	// 총 크기: 16 bytes per track
};

// 애니메이션 트랙 (CHUNK_ANIM_TRACK)
struct AnimTrackHeader {
	uint32_t bone_index;      // 본 인덱스
	uint32_t num_keys;        // 키프레임 개수
	uint8_t key_format;       // 비트플래그: POS_COMPRESSED, ROT_COMPRESSED, SCL_COMPRESSED
	uint8_t time_format;      // 시간 저장 포맷 (0=uint16 normalized, 1=float)
	uint16_t reserved;        // 예약됨
	// 총 크기: 12 bytes
};

// 키프레임 포맷 플래그
enum KeyFormat : uint8_t {
	KEY_POS_COMPRESSED = 0x01,
	KEY_ROT_COMPRESSED = 0x02,
	KEY_SCL_COMPRESSED = 0x04,
	KEY_UNIFORM_SAMPLING = 0x08
};
#pragma pack(pop)

// 청크 타입 열거형 (업데이트된 포맷)
enum ChunkType : uint32_t {
	CHUNK_META = 0x01,
	CHUNK_VERTEX_STREAM = 0x02,
	CHUNK_INDEX_STREAM = 0x03,
	CHUNK_MESH_SECTIONS = 0x04,
	CHUNK_SKELETON = 0x05,
	CHUNK_SKINNING = 0x06,
	CHUNK_ANIMATION_INDEX = 0x07,
	CHUNK_ANIM_TRACK = 0x08,
	CHUNK_MATERIALS = 0x09,
	CHUNK_BOUNDING = 0x0A,
	CHUNK_LOD = 0x0B,
	CHUNK_CUSTOM = 0xFF
};

// 런타임 본 정보
struct BoneInfo {
	std::string name;
	int32_t parent_index;
	glm::mat4 local_bind;   // bind pose local transform
	glm::mat4 inv_bind;     // inverse bind matrix
	uint32_t flags;
};

// 런타임 애니메이션 키프레임
struct AnimationKeyframe {
	float timestamp;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
};

// 런타임 애니메이션 트랙
struct AnimationTrack {
	uint32_t bone_index;
	std::vector<AnimationKeyframe> keyframes;
	uint8_t key_format;
	uint8_t time_format;
};

// 런타임 애니메이션 클립
struct AnimationClip {
	std::string name;
	float duration;
	float sample_rate;
	std::vector<AnimationTrack> tracks;
};

// XMesh 런타임 데이터
struct XMeshData {
	std::string name;

	// VAO (로드 시 1회만 설정)
	GLuint vao{};

	// 스트림별 VBO
	std::vector<GLuint> vbos;
	GLuint ebo{};

	// 스트림 정보
	struct StreamInfo {
		const void* data;
		size_t size;
		uint32_t element_size;
		uint32_t count;
		uint32_t stream_id;
	};
	std::vector<StreamInfo> streams;

	// 인덱스 정보
	const void* index_data{};
	size_t index_size{};
	uint32_t index_count{};
	uint32_t index_type{}; // GL_UNSIGNED_SHORT or GL_UNSIGNED_INT

	// 메시 섹션
	std::vector<MeshSection> sections;

	// 스켈레톤
	std::vector<BoneInfo> bones;
	std::unordered_map<std::string, uint32_t> bone_name_to_index;
	bool has_skeleton{};

	// 애니메이션
	std::vector<AnimationClip> animations;
	std::unordered_map<std::string, uint32_t> anim_name_to_index;

	// 스키닝 스트림 (bone indices, weights)
	bool has_skinning{};

	// 원본 파일 데이터 (메모리 관리용)
	std::vector<uint8_t> file_buffer;
};

// 애니메이션 재생 상태
struct AnimationState {
	const AnimationClip* current_clip{};
	float current_time{};
	bool is_playing{};
	bool is_looping{};
	float playback_speed{1.0f};

	std::vector<glm::mat4> bone_transforms; // 현재 본 변환 행렬
	std::vector<glm::mat4> final_transforms; // 최종 변환 행렬 (GPU 업로드용)
};

class ResourceManager
{
public:
	ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;


	void Active();
	void Deactive();

	bool LoadObj(const std::string_view&, const std::filesystem::path&);
	bool LoadXMesh(const std::string_view&, const std::filesystem::path&);
	bool LoadTexture(const std::string_view&, const std::filesystem::path&);

	GLuint GetVAO() const { return VAO; }
	const ObjData* GetObjData(const std::string_view&) const;
	const XMeshData* GetXMeshData(const std::string_view&) const;
	GLuint GetTexture(const std::string_view&) const;

	// 애니메이션 트랙 디코딩 헬퍼
	bool DecodeAnimationTrack(const XMeshData* mesh, uint32_t clipIndex, uint32_t trackIndex, AnimationTrack& outTrack);

private:
	void SortData();

	static ResourceManager* onceInstance;
	GLuint VAO{};
	std::vector<ObjData> dataList;
	std::vector<XMeshData> xmeshList;
	std::unordered_map<std::string, GLuint> textureMap;
};

