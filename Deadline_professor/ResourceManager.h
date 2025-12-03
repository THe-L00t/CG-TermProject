#pragma once
#include "TotalHeader.h"
#include "FBXCommon.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <string_view>

// ============================================
// 기본 정점 구조체
// ============================================
struct Vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
	glm::ivec4 boneIDs;
	glm::vec4 boneWeights;

	Vertex() noexcept
		: position(0.0f), texcoord(0.0f), normal(0.0f),
		  boneIDs(-1), boneWeights(0.0f) {}
};

// ============================================
// OBJ 데이터 구조체
// ============================================
struct ObjData {
	std::string name;
	GLuint VBO{};
	GLuint EBO{};
	size_t indexCount{};
};

// ============================================
// FBX 스켈레탈 애니메이션 구조체
// ============================================

// 본 정보
struct FBXBone {
	std::string name;
	int parentIndex{ -1 };
	glm::mat4 offsetMatrix{ 1.0f };    // Inverse bind pose
	glm::mat4 nodeTransform{ 1.0f };   // Bind pose (초기 로컬 변환)

	FBXBone() = default;
	FBXBone(const std::string& n, int parent = -1)
		: name(n), parentIndex(parent) {}
};

// 애니메이션 키프레임
struct FBXKeyframe {
	float time{ 0.0f };
	glm::vec3 position{ 0.0f };
	glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };  // identity
	glm::vec3 scale{ 1.0f };

	FBXKeyframe() = default;
};

// 본별 애니메이션 채널
struct FBXAnimChannel {
	std::string boneName;
	std::vector<FBXKeyframe> keyframes;

	FBXAnimChannel() = default;
	explicit FBXAnimChannel(const std::string& name) : boneName(name) {
		keyframes.reserve(64);  // 일반적인 애니메이션 키프레임 수
	}
};

// 애니메이션 클립
struct FBXAnimation {
	std::string name;
	float duration{ 0.0f };
	float ticksPerSecond{ 25.0f };
	std::vector<FBXAnimChannel> channels;

	FBXAnimation() = default;
	explicit FBXAnimation(const std::string& n) : name(n) {
		channels.reserve(32);  // 일반적인 본 개수
	}
};

// FBX 메시
struct FBXMesh {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	GLuint VAO{};
	GLuint VBO{};
	GLuint EBO{};

	FBXMesh() = default;
	~FBXMesh() = default;

	// Move semantics
	FBXMesh(FBXMesh&& other) noexcept
		: vertices(std::move(other.vertices)),
		  indices(std::move(other.indices)),
		  VAO(other.VAO), VBO(other.VBO), EBO(other.EBO)
	{
		other.VAO = other.VBO = other.EBO = 0;
	}

	FBXMesh& operator=(FBXMesh&& other) noexcept {
		if (this != &other) {
			vertices = std::move(other.vertices);
			indices = std::move(other.indices);
			VAO = other.VAO;
			VBO = other.VBO;
			EBO = other.EBO;
			other.VAO = other.VBO = other.EBO = 0;
		}
		return *this;
	}

	// 복사 방지
	FBXMesh(const FBXMesh&) = delete;
	FBXMesh& operator=(const FBXMesh&) = delete;
};

// FBX 모델
struct FBXModel {
	std::string name;
	std::vector<FBXMesh> meshes;
	glm::vec3 boundingBoxMin{ FLT_MAX };
	glm::vec3 boundingBoxMax{ -FLT_MAX };

	// 스켈레탈 애니메이션 데이터
	std::vector<FBXBone> bones;
	std::unordered_map<std::string, int> boneMap;  // 본 이름 → 인덱스
	std::vector<FBXAnimation> animations;
	glm::mat4 globalInverseTransform{ 1.0f };

	FBXModel() = default;
	explicit FBXModel(const std::string& n) : name(n) {
		meshes.reserve(4);
		bones.reserve(64);
		animations.reserve(4);
	}

	// Move semantics
	FBXModel(FBXModel&&) noexcept = default;
	FBXModel& operator=(FBXModel&&) noexcept = default;

	// 복사 방지 (GPU 리소스 포함)
	FBXModel(const FBXModel&) = delete;
	FBXModel& operator=(const FBXModel&) = delete;
};

// ============================================
// ResourceManager 클래스
// ============================================
class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	void Active();
	void Deactive();

	// 리소스 로딩
	bool LoadObj(const std::string_view& name, const std::filesystem::path& path);
	bool LoadFBX(const std::string_view& name, const std::filesystem::path& path);
	bool LoadTexture(const std::string_view& name, const std::filesystem::path& path);

	// 리소스 접근 (const 보장)
	GLuint GetVAO() const noexcept { return VAO; }
	const ObjData* GetObjData(const std::string_view& name) const noexcept;
	const FBXModel* GetFBXModel(const std::string_view& name) const noexcept;
	GLuint GetTexture(const std::string_view& name) const noexcept;

private:
	// 헬퍼 함수들
	void SortData();

	// FBX 로딩 내부 함수
	void ProcessNode(aiNode* node, const aiScene* scene, FBXModel& model);
	FBXMesh ProcessMesh(aiMesh* mesh, const aiScene* scene, FBXModel& model);
	void LoadBones(const aiScene* scene, FBXModel& model);
	void LoadAnimations(const aiScene* scene, FBXModel& model);

	// 정적 인스턴스 (싱글톤 패턴)
	static ResourceManager* onceInstance;

	// 데이터 저장소
	GLuint VAO{};
	std::vector<ObjData> dataList;
	std::vector<FBXModel> fbxModels;
	std::unordered_map<std::string, GLuint> textureMap;
};
