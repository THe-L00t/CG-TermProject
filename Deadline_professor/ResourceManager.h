#pragma once
#include "TotalHeader.h"

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

// XMesh 파일 포맷 구조체
#pragma pack(push, 1)
struct XMeshHeader {
	char magic[6];            // "XMESH\0"
	uint16_t version;         // 버전 (예: 1)
	uint32_t chunk_count;     // 청크 개수
	uint64_t chunk_table_offset; // 청크 테이블 오프셋
	uint32_t flags;           // 플래그 (엔디안, quantization 옵션)
	uint32_t reserved;        // 예약
	uint64_t file_size;       // 파일 크기
	uint8_t uuid[16];         // 고유 ID
};

struct ChunkEntry {
	uint32_t chunk_type;      // 청크 타입
	uint64_t chunk_offset;    // 파일 시작부터의 오프셋
	uint64_t compressed_size; // 압축된 크기 (0이면 압축 안 됨)
	uint64_t decompressed_size; // 압축 해제된 크기
	uint32_t compression;     // 0=none, 1=LZ4, 2=ZSTD, 3=MESHOPT
	uint32_t flags;           // 청크별 플래그
};

struct VertexStreamHeader {
	uint32_t stream_id;      // 0=pos, 1=normal, 2=uv0, 3=uv1, 4=color, 5=weights, 6=bones
	uint32_t element_size;   // 요소당 바이트 크기
	uint32_t count;          // 요소 개수
	uint32_t stride;         // stride (0이면 tightly packed)
	uint32_t format_flags;   // quantization 정보
	uint32_t reserved;
};

struct IndexStreamHeader {
	uint32_t index_type;     // 16 또는 32 bit
	uint32_t index_count;    // 인덱스 개수
	uint32_t reserved[2];
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

// 스켈레톤 구조체
struct BoneInfo {
	char name[64];           // 본 이름
	int32_t parent_index;    // 부모 본 인덱스 (-1이면 루트)
	glm::mat4 offset_matrix; // 본 오프셋 행렬 (inverse bind pose)
	glm::mat4 local_transform; // 로컬 변환 행렬
};

struct SkeletonHeader {
	uint32_t bone_count;
	uint32_t reserved[3];
};

// 애니메이션 구조체
struct AnimationKeyframe {
	float timestamp;         // 키프레임 시간
	glm::vec3 position;      // 위치
	glm::quat rotation;      // 회전 (quaternion)
	glm::vec3 scale;         // 스케일
};

struct AnimationTrack {
	uint32_t bone_index;     // 대상 본 인덱스
	uint32_t keyframe_count; // 키프레임 개수
	std::vector<AnimationKeyframe> keyframes;
};

struct AnimationClip {
	char name[64];           // 애니메이션 이름
	float duration;          // 전체 재생 시간
	float ticks_per_second;  // 초당 틱
	std::vector<AnimationTrack> tracks;
};

struct AnimationHeader {
	char name[64];
	float duration;
	float ticks_per_second;
	uint32_t track_count;
	uint32_t reserved[2];
};

struct TrackHeader {
	uint32_t bone_index;
	uint32_t keyframe_count;
	uint32_t reserved[2];
};
#pragma pack(pop)

// 청크 타입 열거형
enum ChunkType : uint32_t {
	CHUNK_META = 0x01,
	CHUNK_VERTEX_STREAM = 0x02,
	CHUNK_INDEX_STREAM = 0x03,
	CHUNK_MESH_SECTIONS = 0x04,
	CHUNK_SKELETON = 0x05,
	CHUNK_ANIMATIONS = 0x06,
	CHUNK_MATERIALS = 0x07,
	CHUNK_BOUNDING = 0x08,
	CHUNK_CUSTOM = 0xFF
};

// XMesh 런타임 데이터
struct XMeshData {
	std::string name;

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
	bool has_skeleton{};

	// 애니메이션
	std::vector<AnimationClip> animations;

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

	GLuint GetVAO() const { return VAO; }
	const ObjData* GetObjData(const std::string_view&) const;
	const XMeshData* GetXMeshData(const std::string_view&) const;

private:
	void SortData();

	static ResourceManager* onceInstance;
	GLuint VAO{};
	std::vector<ObjData> dataList;
	std::vector<XMeshData> xmeshList;
};

