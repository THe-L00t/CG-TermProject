#pragma once
#include "TotalHeader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
	glm::ivec4 boneIDs;      // 최대 4개 본 인덱스
	glm::vec4 boneWeights;   // 최대 4개 본 가중치

	Vertex() : position(0.0f), texcoord(0.0f), normal(0.0f),
	           boneIDs(-1), boneWeights(0.0f) {}
};

struct ObjData {
	std::string name;
	GLuint VBO{};
	GLuint EBO{};
	size_t indexCount{};
};

// FBX 본 정보
struct FBXBone {
	std::string name;
	int parentIndex;           // 부모 본 인덱스 (-1이면 루트)
	glm::mat4 offsetMatrix;    // 본 오프셋 행렬 (inverse bind pose)
	glm::mat4 finalTransform;  // 최종 변환 행렬
};

// FBX 애니메이션 키프레임
struct FBXKeyframe {
	float time;
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
};

// FBX 애니메이션 채널 (본별 애니메이션)
struct FBXAnimChannel {
	std::string boneName;
	std::vector<FBXKeyframe> keyframes;
};

// FBX 애니메이션 클립
struct FBXAnimation {
	std::string name;
	float duration;           // 초 단위
	float ticksPerSecond;     // FPS
	std::vector<FBXAnimChannel> channels;
};

// FBX 모델 데이터
struct FBXMesh {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	GLuint VAO{};
	GLuint VBO{};
	GLuint EBO{};
};

struct FBXModel {
	std::string name;
	std::vector<FBXMesh> meshes;
	glm::vec3 boundingBoxMin;
	glm::vec3 boundingBoxMax;

	// 스켈레탈 애니메이션 데이터
	std::vector<FBXBone> bones;
	std::unordered_map<std::string, int> boneMap; // 본 이름 -> 인덱스
	std::vector<FBXAnimation> animations;
	glm::mat4 globalInverseTransform;             // 루트 역변환
};

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;

	void Active();
	void Deactive();

	bool LoadObj(const std::string_view&, const std::filesystem::path&);
	bool LoadFBX(const std::string_view&, const std::filesystem::path&);
	bool LoadTexture(const std::string_view&, const std::filesystem::path&);

	GLuint GetVAO() const { return VAO; }
	const ObjData* GetObjData(const std::string_view&) const;
	const FBXModel* GetFBXModel(const std::string_view&) const;
	GLuint GetTexture(const std::string_view&) const;

private:
	void SortData();
	void ProcessNode(aiNode* node, const aiScene* scene, FBXModel& model);
	FBXMesh ProcessMesh(aiMesh* mesh, const aiScene* scene, FBXModel& model);
	void LoadBones(const aiScene* scene, FBXModel& model);
	void LoadAnimations(const aiScene* scene, FBXModel& model);

	static ResourceManager* onceInstance;
	GLuint VAO{};
	std::vector<ObjData> dataList;
	std::vector<FBXModel> fbxModels;
	std::unordered_map<std::string, GLuint> textureMap;
};

