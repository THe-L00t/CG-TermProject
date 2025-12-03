#include "ResourceManager.h"
#include <set>
#include <functional>
#include <algorithm>
#include <array>

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
	const aiScene* scene = importer.ReadFile(path.string(),
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights      // 최대 4개 본 가중치
	);

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
		}

		fbxMesh.vertices.push_back(std::move(vertex));
	}

	// 본 가중치 처리
	if (mesh->HasBones()) {
		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			const aiBone* bone = mesh->mBones[boneIndex];
			std::string boneName(bone->mName.C_Str());

			// 본 ID 찾기
			auto it = model.boneMap.find(boneName);
			if (it == model.boneMap.end()) {
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
				for (int i = 0; i < 4; ++i) {
					if (vertex.boneWeights[i] == 0.0f) {
						vertex.boneIDs[i] = boneID;
						vertex.boneWeights[i] = weight;
						break;
					}
				}
			}
		}
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
	// 모든 메시의 본 정보 수집
	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		const aiMesh* mesh = scene->mMeshes[meshIndex];
		if (!mesh->HasBones()) continue;

		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			const aiBone* bone = mesh->mBones[boneIndex];
			std::string boneName(bone->mName.C_Str());

			// 이미 추가된 본인지 확인
			if (model.boneMap.find(boneName) != model.boneMap.end()) {
				continue;
			}

			// 새 본 추가
			int newBoneIndex = static_cast<int>(model.bones.size());
			model.boneMap[boneName] = newBoneIndex;

			FBXBone fbxBone(boneName);
			// Offset matrix를 FBX → OpenGL 좌표계로 변환
			fbxBone.offsetMatrix = ConvertAiMatToGlmFixed(bone->mOffsetMatrix);

			model.bones.push_back(std::move(fbxBone));
		}
	}

	// 본 계층 구조 빌드 (람다 재귀 사용)
	std::function<void(aiNode*, int)> buildHierarchy = [&](aiNode* node, int parentIndex) {
		std::string nodeName(node->mName.C_Str());

		auto it = model.boneMap.find(nodeName);
		if (it != model.boneMap.end()) {
			int currentBoneIndex = it->second;
			model.bones[currentBoneIndex].parentIndex = parentIndex;

			// 노드 변환 저장 (bind pose) - FBX → OpenGL 좌표계 변환
			model.bones[currentBoneIndex].nodeTransform = ConvertAiMatToGlmFixed(node->mTransformation);

			parentIndex = currentBoneIndex;
		}

		// 자식 노드 재귀 처리
		for (unsigned int i = 0; i < node->mNumChildren; ++i) {
			buildHierarchy(node->mChildren[i], parentIndex);
		}
	};

	buildHierarchy(scene->mRootNode, -1);
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
	// Texture loading implementation (기존 코드 유지)
	return false;
}
