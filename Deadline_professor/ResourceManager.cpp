#include "ResourceManager.h"
#include <set>
#include <functional>
#include <algorithm>
#include <array>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ============================================
// ResourceManager 구현
// ============================================
ResourceManager* ResourceManager::onceInstance = nullptr;

ResourceManager::ResourceManager()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
}

ResourceManager::~ResourceManager()
{
	// FBX 모델 정리
	for (auto& model : fbxModels) {
		for (auto& mesh : model.meshes) {
			if (mesh.VAO) glDeleteVertexArrays(1, &mesh.VAO);
			if (mesh.VBO) glDeleteBuffers(1, &mesh.VBO);
			if (mesh.EBO) glDeleteBuffers(1, &mesh.EBO);
		}
	}

	// OBJ 데이터 정리
	for (auto& obj : dataList) {
		if (obj.VBO) glDeleteBuffers(1, &obj.VBO);
		if (obj.EBO) glDeleteBuffers(1, &obj.EBO);
	}

	if (VAO) glDeleteVertexArrays(1, &VAO);

	// 텍스처 정리
	for (auto& [name, texID] : textureMap) {
		if (texID) glDeleteTextures(1, &texID);
	}
}

void ResourceManager::Active()
{
	onceInstance = this;
}

void ResourceManager::Deactive()
{
	if (onceInstance == this) {
		onceInstance = nullptr;
	}
}

// ============================================
// 리소스 접근 함수
// ============================================
const ObjData* ResourceManager::GetObjData(const std::string_view& name) const noexcept
{
	std::string nameStr(name);
	auto it = std::find_if(dataList.begin(), dataList.end(),
		[&nameStr](const ObjData& data) { return data.name == nameStr; });
	return it != dataList.end() ? &(*it) : nullptr;
}

const FBXModel* ResourceManager::GetFBXModel(const std::string_view& name) const noexcept
{
	std::string nameStr(name);
	auto it = std::find_if(fbxModels.begin(), fbxModels.end(),
		[&nameStr](const FBXModel& model) { return model.name == nameStr; });
	return it != fbxModels.end() ? &(*it) : nullptr;
}

GLuint ResourceManager::GetTexture(const std::string_view& name) const noexcept
{
	std::string nameStr(name);
	auto it = textureMap.find(nameStr);
	return it != textureMap.end() ? it->second : 0;
}

// ============================================
// FBX 로딩
// ============================================
bool ResourceManager::LoadFBX(const std::string_view& name, const std::filesystem::path& path)
{
	Assimp::Importer importer;

	// FBX 파일 로드 (최적화 플래그 적용)
	unsigned int flags = aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights;      // 최대 4개 본 가중치

	std::cout << "Loading FBX with flags: " << flags << std::endl;
	std::cout << "aiProcess_MakeLeftHanded: " << ((flags & aiProcess_MakeLeftHanded) ? "YES" : "NO") << std::endl;
	std::cout << "aiProcess_FlipUVs: " << ((flags & aiProcess_FlipUVs) ? "YES" : "NO") << std::endl;
	std::cout << "aiProcess_FlipWindingOrder: " << ((flags & aiProcess_FlipWindingOrder) ? "YES" : "NO") << std::endl;

	const aiScene* scene = importer.ReadFile(path.string(), flags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "ERROR::ASSIMP: " << importer.GetErrorString() << std::endl;
		return false;
	}

	// FBXModel 생성 (예약으로 메모리 최적화)
	FBXModel model;
	model.name = std::string(name);
	model.meshes.reserve(4);
	model.bones.reserve(64);
	model.animations.reserve(4);

	// 글로벌 역변환 행렬 저장 (FBX → OpenGL 좌표계 변환)
	aiMatrix4x4 globalTransform = scene->mRootNode->mTransformation;
	globalTransform.Inverse();
	model.globalInverseTransform = ConvertAiMatToGlmFixed(globalTransform);

	// 본과 애니메이션 먼저 로드 (메시에서 참조하므로)
	LoadBones(scene, model);
	LoadAnimations(scene, model);

	// 씬 노드 처리
	ProcessNode(scene->mRootNode, scene, model);

	// ============================================
	// DEBUG: FBX 로드 후 변환 검증
	// ============================================
	std::cout << "==== FBX Debug Dump ====" << std::endl;
	std::cout << "Model: " << model.name << std::endl;
	std::cout << "globalInverseTransform determinant: " << glm::determinant(model.globalInverseTransform) << std::endl;

	if (!model.bones.empty()) {
		const auto& b = model.bones[0];
		std::cout << "First bone name: " << b.name << " parent: " << b.parentIndex << std::endl;
		std::cout << "nodeTransform (col-major):\n";
		const glm::mat4& nt = b.nodeTransform;
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) std::cout << nt[c][r] << " ";
			std::cout << "\n";
		}
		std::cout << "offsetMatrix (col-major):\n";
		const glm::mat4& om = b.offsetMatrix;
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) std::cout << om[c][r] << " ";
			std::cout << "\n";
		}
		glm::mat4 sampleGlobal = nt; // as if root
		glm::mat4 final = model.globalInverseTransform * sampleGlobal * om;
		std::cout << "sample final matrix det: " << glm::determinant(final) << std::endl;
	}
	else {
		std::cout << "No bones!" << std::endl;
	}

	if (!model.meshes.empty() && !model.meshes[0].vertices.empty()) {
		auto& v = model.meshes[0].vertices[0];
		std::cout << "Sample vertex pos: " << v.position.x << ", " << v.position.y << ", " << v.position.z << std::endl;
		std::cout << "Sample vertex boneIDs: " << v.boneIDs[0] << "," << v.boneIDs[1] << "," << v.boneIDs[2] << "," << v.boneIDs[3] << std::endl;
		std::cout << "Sample vertex weights: " << v.boneWeights[0] << "," << v.boneWeights[1] << "," << v.boneWeights[2] << "," << v.boneWeights[3] << std::endl;
	}
	std::cout << "=========================" << std::endl;

	fbxModels.push_back(std::move(model));
	return true;
}

void ResourceManager::ProcessNode(aiNode* node, const aiScene* scene, FBXModel& model)
{
	// 이 노드의 모든 메시 처리
	model.meshes.reserve(model.meshes.size() + node->mNumMeshes);
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		model.meshes.push_back(ProcessMesh(mesh, scene, model));
	}

	// 자식 노드 재귀 처리
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		ProcessNode(node->mChildren[i], scene, model);
	}
}

FBXMesh ResourceManager::ProcessMesh(aiMesh* mesh, const aiScene* scene, FBXModel& model)
{
	FBXMesh fbxMesh;

	// UV 좌표 존재 여부 로그
	bool hasUVs = mesh->mTextureCoords[0] != nullptr;
	std::cout << "Mesh '" << mesh->mName.C_Str() << "' has UV coordinates: " << (hasUVs ? "YES" : "NO") << std::endl;

	// 메모리 미리 할당 (성능 최적화)
	fbxMesh.vertices.reserve(mesh->mNumVertices);
	fbxMesh.indices.reserve(mesh->mNumFaces * 3);

	// 정점 데이터 처리 (FBX → OpenGL 좌표계 변환)
	glm::mat4 fix = AxisFixMatrix();

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		Vertex vertex;

		// Position - Axis fix 적용
		glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		glm::vec4 p4 = fix * glm::vec4(pos, 1.0f);
		vertex.position = glm::vec3(p4);

		// 바운딩 박스 업데이트
		model.boundingBoxMin = glm::min(model.boundingBoxMin, vertex.position);
		model.boundingBoxMax = glm::max(model.boundingBoxMax, vertex.position);

		// Normal - Axis fix 적용
		if (mesh->HasNormals()) {
			glm::vec3 n(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			glm::vec4 n4 = fix * glm::vec4(n, 0.0f);
			vertex.normal = glm::normalize(glm::vec3(n4));
		}

		// Texture coordinates
		if (mesh->mTextureCoords[0]) {
			vertex.texcoord = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		} else {
			// UV 좌표가 없으면 기본값 설정
			vertex.texcoord = glm::vec2(0.0f, 0.0f);
		}

		fbxMesh.vertices.push_back(std::move(vertex));
	}

	// 본 가중치 처리
	if (mesh->HasBones()) {
		int missingBones = 0;
		int totalWeightsApplied = 0;

		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			const aiBone* bone = mesh->mBones[boneIndex];
			std::string boneName(bone->mName.C_Str());

			// 본 ID 찾기
			auto it = model.boneMap.find(boneName);
			if (it == model.boneMap.end()) {
				missingBones++;
				std::cerr << "WARNING: Bone not found in boneMap: " << boneName << std::endl;
				continue;
			}
			int boneID = it->second;

			// 가중치 적용
			for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				unsigned int vertexID = bone->mWeights[weightIndex].mVertexId;
				float weight = bone->mWeights[weightIndex].mWeight;

				if (vertexID >= fbxMesh.vertices.size()) continue;

				Vertex& vertex = fbxMesh.vertices[vertexID];
				// 빈 슬롯에 가중치 추가
				bool added = false;
				for (int i = 0; i < 4; ++i) {
					if (vertex.boneWeights[i] == 0.0f) {
						vertex.boneIDs[i] = boneID;
						vertex.boneWeights[i] = weight;
						added = true;
						totalWeightsApplied++;
						break;
					}
				}
				if (!added) {
					std::cerr << "WARNING: Vertex " << vertexID << " has more than 4 bone weights!" << std::endl;
				}
			}
		}

		// 가중치 정규화 및 검증
		int zeroWeightCount = 0;
		for (size_t i = 0; i < fbxMesh.vertices.size(); ++i) {
			auto& vertex = fbxMesh.vertices[i];
			float totalWeight = vertex.boneWeights[0] + vertex.boneWeights[1] +
			                    vertex.boneWeights[2] + vertex.boneWeights[3];

			if (totalWeight > 0.0001f) {
				// 정규화
				vertex.boneWeights[0] /= totalWeight;
				vertex.boneWeights[1] /= totalWeight;
				vertex.boneWeights[2] /= totalWeight;
				vertex.boneWeights[3] /= totalWeight;
			}
			else {
				// 가중치가 없는 정점 - 루트 본에 바인딩
				zeroWeightCount++;
				vertex.boneIDs[0] = 0;  // 루트 본
				vertex.boneWeights[0] = 1.0f;
			}
		}

		if (missingBones > 0) {
			std::cout << "Mesh has " << missingBones << " missing bones in boneMap" << std::endl;
		}
		if (zeroWeightCount > 0) {
			std::cout << "Mesh has " << zeroWeightCount << " vertices with zero weights (bound to root)" << std::endl;
		}
		std::cout << "Total bone weights applied: " << totalWeightsApplied << std::endl;
	}

	// 인덱스 데이터 처리
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		const aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j) {
			fbxMesh.indices.push_back(face.mIndices[j]);
		}
	}

	// 본 가중치 정규화 (합이 1.0이 되도록)
	for (auto& vertex : fbxMesh.vertices) {
		float totalWeight = vertex.boneWeights[0] + vertex.boneWeights[1] +
		                    vertex.boneWeights[2] + vertex.boneWeights[3];

		if (totalWeight > 0.001f) {
			vertex.boneWeights /= totalWeight;
		}
	}

	// GPU 버퍼 생성
	glGenVertexArrays(1, &fbxMesh.VAO);
	glGenBuffers(1, &fbxMesh.VBO);
	glGenBuffers(1, &fbxMesh.EBO);

	glBindVertexArray(fbxMesh.VAO);

	// VBO 설정
	glBindBuffer(GL_ARRAY_BUFFER, fbxMesh.VBO);
	glBufferData(GL_ARRAY_BUFFER,
		fbxMesh.vertices.size() * sizeof(Vertex),
		fbxMesh.vertices.data(),
		GL_STATIC_DRAW);

	// EBO 설정
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbxMesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		fbxMesh.indices.size() * sizeof(unsigned int),
		fbxMesh.indices.data(),
		GL_STATIC_DRAW);

	// 정점 속성 설정
	// Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	// Texcoord
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

	// Normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	// Bone IDs
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, boneIDs));

	// Bone Weights
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, boneWeights));

	glBindVertexArray(0);

	return fbxMesh;
}

void ResourceManager::LoadBones(const aiScene* scene, FBXModel& model)
{
	// ============================================
	// STEP 1: 메시의 본부터 먼저 등록 (offsetMatrix 포함)
	// 이렇게 해야 정점의 boneID와 boneMap 인덱스가 일치
	// ============================================
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		const aiMesh* mesh = scene->mMeshes[meshIndex];
		if (!mesh->HasBones()) continue;

		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			const aiBone* bone = mesh->mBones[boneIndex];
			std::string boneName(bone->mName.C_Str());

			// 아직 등록되지 않은 본이면 추가
			if (model.boneMap.find(boneName) == model.boneMap.end()) {
				int index = static_cast<int>(model.bones.size());
				model.boneMap[boneName] = index;

				FBXBone fbxBone;
				fbxBone.name = boneName;
				fbxBone.offsetMatrix = ConvertAiMatToGlmFixed(bone->mOffsetMatrix);
				fbxBone.nodeTransform = glm::mat4(1.0f); // 나중에 업데이트

				model.bones.push_back(std::move(fbxBone));
			}
		}
	}

	// ============================================
	// STEP 2: 씬 트리의 모든 노드를 순회하며 nodeTransform 설정
	// 및 누락된 본 추가
	// ============================================
	std::function<void(aiNode*)> collectAllNodes = [&](aiNode* node) {
		std::string nodeName(node->mName.C_Str());

		// 아직 등록되지 않은 노드면 본으로 등록
		if (model.boneMap.find(nodeName) == model.boneMap.end()) {
			int index = static_cast<int>(model.bones.size());
			model.boneMap[nodeName] = index;

			FBXBone bone;
			bone.name = nodeName;
			bone.nodeTransform = ConvertAiMatToGlmFixed(node->mTransformation);
			bone.offsetMatrix = glm::mat4(1.0f); // 스킨에 영향 없는 본

			model.bones.push_back(std::move(bone));
		}
		else {
			// 이미 등록된 본이면 nodeTransform만 업데이트
			int index = model.boneMap[nodeName];
			model.bones[index].nodeTransform = ConvertAiMatToGlmFixed(node->mTransformation);
		}

		// 자식 노드 재귀
		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			collectAllNodes(node->mChildren[i]);
		}
	};

	collectAllNodes(scene->mRootNode);

	// ============================================
	// STEP 3: 애니메이션 채널에 등장하는 본 이름 확인 및 등록
	// ============================================
	for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
		const aiAnimation* anim = scene->mAnimations[animIndex];
		for (unsigned int channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex) {
			const aiNodeAnim* channel = anim->mChannels[channelIndex];
			std::string boneName(channel->mNodeName.C_Str());

			// Assimp 접미사 제거
			size_t suffixPos = boneName.find("_$AssimpFbx$_");
			if (suffixPos != std::string::npos) {
				boneName = boneName.substr(0, suffixPos);
			}

			// 아직 등록되지 않은 본이면 추가 (애니메이션용)
			if (model.boneMap.find(boneName) == model.boneMap.end()) {
				int index = static_cast<int>(model.bones.size());
				model.boneMap[boneName] = index;

				FBXBone bone;
				bone.name = boneName;
				bone.nodeTransform = glm::mat4(1.0f);
				bone.offsetMatrix = glm::mat4(1.0f);

				model.bones.push_back(std::move(bone));

				std::cout << "Warning: Animation bone not in scene tree: " << boneName << std::endl;
			}
		}
	}

	// ============================================
	// STEP 4: 계층 구조 설정 (parentIndex)
	// ============================================
	std::function<void(aiNode*, int)> setParentIndices = [&](aiNode* node, int parentIndex) {
		std::string nodeName(node->mName.C_Str());

		auto it = model.boneMap.find(nodeName);
		if (it != model.boneMap.end()) {
			int currentIndex = it->second;
			model.bones[currentIndex].parentIndex = parentIndex;

			// 현재 본을 부모로 설정
			parentIndex = currentIndex;
		}

		// 자식 노드 재귀
		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			setParentIndices(node->mChildren[i], parentIndex);
		}
	};

	setParentIndices(scene->mRootNode, -1);

	// ============================================
	// STEP 5: offsetMatrix가 없는 본 검증 및 통계
	// ============================================
	int bonesWithOffset = 0;
	int bonesWithoutOffset = 0;
	for (size_t i = 0; i < model.bones.size(); ++i) {
		const auto& bone = model.bones[i];
		// offsetMatrix가 항등 행렬인지 확인
		bool isIdentity = true;
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				float expected = (r == c) ? 1.0f : 0.0f;
				if (std::abs(bone.offsetMatrix[c][r] - expected) > 0.0001f) {
					isIdentity = false;
					break;
				}
			}
			if (!isIdentity) break;
		}

		if (isIdentity) {
			bonesWithoutOffset++;
		}
		else {
			bonesWithOffset++;
		}
	}

	std::cout << "Total bones registered: " << model.bones.size() << std::endl;
	std::cout << "  Bones with offsetMatrix: " << bonesWithOffset << std::endl;
	std::cout << "  Bones without offsetMatrix (identity): " << bonesWithoutOffset << std::endl;
}

void ResourceManager::LoadAnimations(const aiScene* scene, FBXModel& model)
{
	for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
		const aiAnimation* anim = scene->mAnimations[animIndex];

		FBXAnimation fbxAnim;
		fbxAnim.name = anim->mName.C_Str();
		fbxAnim.ticksPerSecond = static_cast<float>(anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0);
		fbxAnim.duration = static_cast<float>(anim->mDuration) / fbxAnim.ticksPerSecond;
		fbxAnim.channels.reserve(32);

		// 각 본의 애니메이션 채널 처리
		for (unsigned int channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex) {
			const aiNodeAnim* channel = anim->mChannels[channelIndex];

			// Assimp 접미사 제거
			std::string boneName = channel->mNodeName.C_Str();
			size_t suffixPos = boneName.find("_$AssimpFbx$_");
			if (suffixPos != std::string::npos) {
				boneName = boneName.substr(0, suffixPos);
			}

			FBXAnimChannel fbxChannel(boneName);

			// 모든 키 타입의 시간 수집
			std::set<float> allTimes;
			for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i) {
				allTimes.insert(static_cast<float>(channel->mPositionKeys[i].mTime));
			}
			for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i) {
				allTimes.insert(static_cast<float>(channel->mRotationKeys[i].mTime));
			}
			for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i) {
				allTimes.insert(static_cast<float>(channel->mScalingKeys[i].mTime));
			}

			// 각 시간에 대한 키프레임 생성
			for (float timeTicks : allTimes) {
				FBXKeyframe keyframe;
				keyframe.time = timeTicks / fbxAnim.ticksPerSecond;

				// TRS 데이터 수집
				aiVector3D pos(0, 0, 0);
				aiQuaternion rot(1, 0, 0, 0);
				aiVector3D scale(1, 1, 1);

				// Position 키 찾기
				if (channel->mNumPositionKeys > 0) {
					unsigned int posIndex = 0;
					for (unsigned int i = 0; i < channel->mNumPositionKeys; ++i) {
						if (channel->mPositionKeys[i].mTime <= timeTicks) {
							posIndex = i;
						}
					}
					pos = channel->mPositionKeys[posIndex].mValue;
				}

				// Rotation 키 찾기
				if (channel->mNumRotationKeys > 0) {
					unsigned int rotIndex = 0;
					for (unsigned int i = 0; i < channel->mNumRotationKeys; ++i) {
						if (channel->mRotationKeys[i].mTime <= timeTicks) {
							rotIndex = i;
						}
					}
					rot = channel->mRotationKeys[rotIndex].mValue;
				}

				// Scale 키 찾기
				if (channel->mNumScalingKeys > 0) {
					unsigned int scaleIndex = 0;
					for (unsigned int i = 0; i < channel->mNumScalingKeys; ++i) {
						if (channel->mScalingKeys[i].mTime <= timeTicks) {
							scaleIndex = i;
						}
					}
					scale = channel->mScalingKeys[scaleIndex].mValue;
				}

				// TRS를 GLM으로 변환
				glm::vec3 glmPos(pos.x, pos.y, pos.z);
				glm::quat glmRot(rot.w, rot.x, rot.y, rot.z);
				glm::vec3 glmScale(scale.x, scale.y, scale.z);

				// FBX 좌표계 TRS → OpenGL 좌표계 TRS 변환
				// fix * TRS * fix 공식 사용
				glm::mat4 fixedTRS = BuildAndFixTRS(glmPos, glmRot, glmScale);

				// 변환된 TRS 분해
				keyframe.position = glm::vec3(fixedTRS[3]); // translation column

				// Scale 추출 (벡터 길이)
				keyframe.scale = glm::vec3(
					glm::length(glm::vec3(fixedTRS[0])),
					glm::length(glm::vec3(fixedTRS[1])),
					glm::length(glm::vec3(fixedTRS[2]))
				);

				// Rotation 추출 (정규화된 회전 행렬에서 쿼터니언)
				glm::mat3 rotMat(
					glm::vec3(fixedTRS[0]) / keyframe.scale.x,
					glm::vec3(fixedTRS[1]) / keyframe.scale.y,
					glm::vec3(fixedTRS[2]) / keyframe.scale.z
				);
				keyframe.rotation = glm::quat_cast(rotMat);

				fbxChannel.keyframes.push_back(std::move(keyframe));
			}

			fbxAnim.channels.push_back(std::move(fbxChannel));
		}

		model.animations.push_back(std::move(fbxAnim));
	}
}

// ============================================
// OBJ 로딩 (기존 코드 유지)
// ============================================
bool ResourceManager::LoadObj(const std::string_view& name, const std::filesystem::path& path)
{
	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_texcoords;
	std::vector<glm::vec3> temp_normals;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	std::ifstream file(path, std::ios::in);
	if (!file) {
		std::cerr << "f-LoadObj failed : " << path << std::endl;
		return false;
	}
	std::string line;

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string type;
		ss >> type;

		if (type == "v") {
			glm::vec3 pos;
			ss >> pos.x >> pos.y >> pos.z;
			temp_positions.push_back(pos);
		}
		else if (type == "vt") {
			glm::vec2 uv;
			ss >> uv.x >> uv.y;
			temp_texcoords.push_back(uv);
		}
		else if (type == "vn") {
			glm::vec3 normal;
			ss >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (type == "f") {
			std::vector<std::string> faceVertices;
			std::string faceVertex;
			while (ss >> faceVertex) {
				faceVertices.push_back(faceVertex);
			}

			auto parseVertex = [&](const std::string& f) -> Vertex {
				int posIdx = 0, uvIdx = 0, norIdx = 0;

				size_t firstSlash = f.find('/');
				if (firstSlash == std::string::npos) {
					posIdx = std::stoi(f);
				}
				else {
					size_t secondSlash = f.find('/', firstSlash + 1);
					posIdx = std::stoi(f.substr(0, firstSlash));

					if (secondSlash != std::string::npos) {
						if (secondSlash != firstSlash + 1) {
							uvIdx = std::stoi(f.substr(firstSlash + 1, secondSlash - firstSlash - 1));
						}
						if (secondSlash + 1 < f.length()) {
							norIdx = std::stoi(f.substr(secondSlash + 1));
						}
					}
					else {
						uvIdx = std::stoi(f.substr(firstSlash + 1));
					}
				}

				Vertex vertex;
				if (posIdx > 0 && posIdx <= temp_positions.size()) {
					vertex.position = temp_positions[posIdx - 1];
				}
				if (uvIdx > 0 && uvIdx <= temp_texcoords.size()) {
					vertex.texcoord = temp_texcoords[uvIdx - 1];
				}
				if (norIdx > 0 && norIdx <= temp_normals.size()) {
					vertex.normal = temp_normals[norIdx - 1];
				}
				return vertex;
			};

			if (faceVertices.size() == 3) {
				for (const auto& fv : faceVertices) {
					vertices.push_back(parseVertex(fv));
					indices.push_back(vertices.size() - 1);
				}
			}
			else if (faceVertices.size() == 4) {
				std::array<Vertex, 4> quad;
				for (int i = 0; i < 4; ++i) {
					quad[i] = parseVertex(faceVertices[i]);
				}
				vertices.insert(vertices.end(), { quad[0], quad[1], quad[2], quad[0], quad[2], quad[3] });
				for (int i = 0; i < 6; ++i) {
					indices.push_back(vertices.size() - 6 + i);
				}
			}
		}
	}

	GLuint VBO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	ObjData objData;
	objData.name = std::string(name);
	objData.VBO = VBO;
	objData.EBO = EBO;
	objData.indexCount = indices.size();

	dataList.push_back(std::move(objData));

	return true;
}

void ResourceManager::SortData()
{
	std::sort(dataList.begin(), dataList.end(),
		[](const ObjData& a, const ObjData& b) { return a.name < b.name; });
}

bool ResourceManager::LoadTexture(const std::string_view& name, const std::filesystem::path& path)
{
	// 이미 로드된 텍스처인지 확인
	std::string keyName(name);
	if (textureMap.find(keyName) != textureMap.end()) {
		std::cout << "Texture '" << name << "' already loaded" << std::endl;
		return true;
	}

	// stb_image를 사용한 이미지 로딩
	int width, height, channels;
	std::cout << "Loading texture from: " << path << std::endl;
	unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

	if (!data) {
		std::cerr << "Failed to load texture: " << path << std::endl;
		std::cerr << "stbi_failure_reason: " << stbi_failure_reason() << std::endl;
		return false;
	}

	std::cout << "Texture loaded: " << width << "x" << height << " with " << channels << " channels" << std::endl;

	// OpenGL 텍스처 생성
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	std::cout << "Generated texture ID: " << textureID << std::endl;

	// 텍스처 파라미터 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 이미지 포맷 결정
	GLenum format = GL_RGB;
	if (channels == 1)
		format = GL_RED;
	else if (channels == 3)
		format = GL_RGB;
	else if (channels == 4)
		format = GL_RGBA;

	// 텍스처 데이터 업로드
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// OpenGL 에러 확인
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "OpenGL Error after texture upload: " << error << std::endl;
		stbi_image_free(data);
		return false;
	}

	// 메모리 해제
	stbi_image_free(data);

	// 텍스처 언바인드
	glBindTexture(GL_TEXTURE_2D, 0);

	// 맵에 저장
	textureMap[keyName] = textureID;
	std::cout << "Texture '" << name << "' successfully stored with ID: " << textureID << std::endl;

	std::cout << "Texture loaded: " << name << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
	return true;
}
