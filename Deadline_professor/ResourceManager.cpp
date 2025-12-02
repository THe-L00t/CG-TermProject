#include "ResourceManager.h"

ResourceManager* ResourceManager::onceInstance = nullptr;

ResourceManager::ResourceManager()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
}

ResourceManager::~ResourceManager()
{
	// Clean up FBX models
	for (auto& model : fbxModels) {
		for (auto& mesh : model.meshes) {
			if (mesh.VAO) glDeleteVertexArrays(1, &mesh.VAO);
			if (mesh.VBO) glDeleteBuffers(1, &mesh.VBO);
			if (mesh.EBO) glDeleteBuffers(1, &mesh.EBO);
		}
	}

	// Clean up OBJ data
	for (auto& obj : dataList) {
		if (obj.VBO) glDeleteBuffers(1, &obj.VBO);
		if (obj.EBO) glDeleteBuffers(1, &obj.EBO);
	}

	if (VAO) glDeleteVertexArrays(1, &VAO);
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

bool ResourceManager::LoadObj(const std::string_view& name, const std::filesystem::path& path)
{
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_texcoords;
    std::vector<glm::vec3> temp_normals;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::ifstream file(path, std::ios::in);
    if (not file) {
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
                else {
                    vertex.texcoord = glm::vec2(0.0f, 0.0f);
                }
                if (norIdx > 0 && norIdx <= temp_normals.size()) {
                    vertex.normal = temp_normals[norIdx - 1];
                }
                else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                return vertex;
            };

            if (faceVertices.size() == 3) {
                for (const auto& fv : faceVertices) {
                    vertices.push_back(parseVertex(fv));
                    indices.push_back(vertices.size() - 1);
                }
            }
            else if (faceVertices.size() > 3) {
                for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
                    vertices.push_back(parseVertex(faceVertices[0]));
                    indices.push_back(vertices.size() - 1);

                    vertices.push_back(parseVertex(faceVertices[i]));
                    indices.push_back(vertices.size() - 1);

                    vertices.push_back(parseVertex(faceVertices[i + 1]));
                    indices.push_back(vertices.size() - 1);
                }
            }
        }
    }

    ObjData temp{};
    temp.name = std::string(name);
    temp.indexCount = indices.size();

    glGenBuffers(1, &(temp.VBO));
    glGenBuffers(1, &(temp.EBO));

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, temp.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, temp.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindVertexArray(0);

    dataList.push_back(temp);

    std::cout << "Loaded OBJ '" << name << "' with " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;

	return true;
}

void ResourceManager::SortData()
{
    sort(dataList.begin(), dataList.end(), [](const ObjData& a, const ObjData& b) {
        return std::lexicographical_compare(a.name.begin(), a.name.end(), b.name.begin(), b.name.end());
        });
}

const ObjData* ResourceManager::GetObjData(const std::string_view& name) const
{
    std::string nameStr(name);
    for (const auto& obj : dataList) {
        if (obj.name == nameStr) {
            return &obj;
        }
    }
    return nullptr;
}

bool ResourceManager::LoadFBX(const std::string_view& name, const std::filesystem::path& path)
{
    std::cout << "Loading FBX file: " << path << std::endl;

    // Assimp Importer 생성
    Assimp::Importer importer;

    // FBX 파일 로드 (삼각형화, 법선 생성, UV 좌표 생성)
    const aiScene* scene = importer.ReadFile(path.string(),
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::cout << "FBX loaded successfully:" << std::endl;
    std::cout << "  Meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "  Materials: " << scene->mNumMaterials << std::endl;
    std::cout << "  Animations: " << scene->mNumAnimations << std::endl;

    // FBXModel 생성
    FBXModel model;
    model.name = std::string(name);
    model.boundingBoxMin = glm::vec3(FLT_MAX);
    model.boundingBoxMax = glm::vec3(-FLT_MAX);

    // 글로벌 역변환 행렬 저장
    aiMatrix4x4 globalTransform = scene->mRootNode->mTransformation;
    globalTransform.Inverse();
    model.globalInverseTransform = glm::mat4(
        globalTransform.a1, globalTransform.b1, globalTransform.c1, globalTransform.d1,
        globalTransform.a2, globalTransform.b2, globalTransform.c2, globalTransform.d2,
        globalTransform.a3, globalTransform.b3, globalTransform.c3, globalTransform.d3,
        globalTransform.a4, globalTransform.b4, globalTransform.c4, globalTransform.d4
    );

    // 본과 애니메이션 로드
    LoadBones(scene, model);
    LoadAnimations(scene, model);

    // 씬 노드 처리
    ProcessNode(scene->mRootNode, scene, model);

    std::cout << "FBX model '" << name << "' loaded with " << model.meshes.size() << " meshes" << std::endl;
    std::cout << "  Bones: " << model.bones.size() << std::endl;
    std::cout << "  Animations: " << model.animations.size() << std::endl;
    std::cout << "  Bounding box: Min(" << model.boundingBoxMin.x << ", " << model.boundingBoxMin.y << ", " << model.boundingBoxMin.z << ")" << std::endl;
    std::cout << "               Max(" << model.boundingBoxMax.x << ", " << model.boundingBoxMax.y << ", " << model.boundingBoxMax.z << ")" << std::endl;

    fbxModels.push_back(std::move(model));
    return true;
}

void ResourceManager::ProcessNode(aiNode* node, const aiScene* scene, FBXModel& model)
{
    // 이 노드의 모든 메시 처리
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

    // 정점 데이터 처리
    fbxMesh.vertices.resize(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex& vertex = fbxMesh.vertices[i];

        // Position
        vertex.position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Normal
        if (mesh->HasNormals()) {
            vertex.normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        } else {
            vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            vertex.texcoord = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        } else {
            vertex.texcoord = glm::vec2(0.0f, 0.0f);
        }
    }

    // 본 가중치 처리
    if (mesh->HasBones()) {
        for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            aiBone* bone = mesh->mBones[boneIndex];
            std::string boneName(bone->mName.C_Str());

            int boneID = -1;
            if (model.boneMap.find(boneName) != model.boneMap.end()) {
                boneID = model.boneMap[boneName];
            }

            // 이 본의 영향을 받는 정점들에 가중치 추가
            for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
                unsigned int vertexID = bone->mWeights[weightIndex].mVertexId;
                float weight = bone->mWeights[weightIndex].mWeight;

                if (vertexID < fbxMesh.vertices.size()) {
                    Vertex& vertex = fbxMesh.vertices[vertexID];
                    // 빈 슬롯 찾아서 추가
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
    }

    // 인덱스 데이터 처리
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            fbxMesh.indices.push_back(face.mIndices[j]);
        }
    }

    // OpenGL 버퍼 생성
    glGenVertexArrays(1, &fbxMesh.VAO);
    glGenBuffers(1, &fbxMesh.VBO);
    glGenBuffers(1, &fbxMesh.EBO);

    glBindVertexArray(fbxMesh.VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, fbxMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, fbxMesh.vertices.size() * sizeof(Vertex),
                 fbxMesh.vertices.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fbxMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, fbxMesh.indices.size() * sizeof(unsigned int),
                 fbxMesh.indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Texture coordinates
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

    std::cout << "  Processed mesh: " << fbxMesh.vertices.size() << " vertices, "
              << fbxMesh.indices.size() << " indices" << std::endl;

    return fbxMesh;
}

const FBXModel* ResourceManager::GetFBXModel(const std::string_view& name) const
{
    std::string nameStr(name);
    for (const auto& model : fbxModels) {
        if (model.name == nameStr) {
            return &model;
        }
    }
    return nullptr;
}

void ResourceManager::LoadBones(const aiScene* scene, FBXModel& model)
{
    // 모든 메시의 본 정보 수집
    for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
        aiMesh* mesh = scene->mMeshes[meshIndex];

        if (!mesh->HasBones()) continue;

        for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
            aiBone* bone = mesh->mBones[boneIndex];
            std::string boneName(bone->mName.C_Str());

            // 이미 추가된 본인지 확인
            if (model.boneMap.find(boneName) != model.boneMap.end()) {
                continue;
            }

            // 새 본 추가
            int newBoneIndex = static_cast<int>(model.bones.size());
            model.boneMap[boneName] = newBoneIndex;

            FBXBone fbxBone;
            fbxBone.name = boneName;
            fbxBone.parentIndex = -1; // 나중에 설정

            // 오프셋 행렬 변환 (Assimp aiMatrix4x4 -> glm::mat4)
            aiMatrix4x4 offset = bone->mOffsetMatrix;
            fbxBone.offsetMatrix = glm::mat4(
                offset.a1, offset.b1, offset.c1, offset.d1,
                offset.a2, offset.b2, offset.c2, offset.d2,
                offset.a3, offset.b3, offset.c3, offset.d3,
                offset.a4, offset.b4, offset.c4, offset.d4
            );

            model.bones.push_back(fbxBone);
        }
    }

    std::cout << "  Loaded " << model.bones.size() << " bones" << std::endl;
}

void ResourceManager::LoadAnimations(const aiScene* scene, FBXModel& model)
{
    for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
        aiAnimation* anim = scene->mAnimations[animIndex];

        FBXAnimation fbxAnim;
        fbxAnim.name = anim->mName.C_Str();
        fbxAnim.duration = static_cast<float>(anim->mDuration);
        fbxAnim.ticksPerSecond = static_cast<float>(anim->mTicksPerSecond != 0 ? anim->mTicksPerSecond : 25.0);

        // 각 본의 애니메이션 채널 처리
        for (unsigned int channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex) {
            aiNodeAnim* channel = anim->mChannels[channelIndex];

            FBXAnimChannel fbxChannel;
            fbxChannel.boneName = channel->mNodeName.C_Str();

            // 키프레임 수 결정 (position, rotation, scale 중 최대값)
            unsigned int numFrames = std::max({
                channel->mNumPositionKeys,
                channel->mNumRotationKeys,
                channel->mNumScalingKeys
            });

            for (unsigned int frameIndex = 0; frameIndex < numFrames; ++frameIndex) {
                FBXKeyframe keyframe;

                // Position
                if (frameIndex < channel->mNumPositionKeys) {
                    keyframe.time = static_cast<float>(channel->mPositionKeys[frameIndex].mTime);
                    keyframe.position = glm::vec3(
                        channel->mPositionKeys[frameIndex].mValue.x,
                        channel->mPositionKeys[frameIndex].mValue.y,
                        channel->mPositionKeys[frameIndex].mValue.z
                    );
                }

                // Rotation
                if (frameIndex < channel->mNumRotationKeys) {
                    aiQuaternion rot = channel->mRotationKeys[frameIndex].mValue;
                    keyframe.rotation = glm::quat(rot.w, rot.x, rot.y, rot.z);
                }

                // Scale
                if (frameIndex < channel->mNumScalingKeys) {
                    keyframe.scale = glm::vec3(
                        channel->mScalingKeys[frameIndex].mValue.x,
                        channel->mScalingKeys[frameIndex].mValue.y,
                        channel->mScalingKeys[frameIndex].mValue.z
                    );
                }

                fbxChannel.keyframes.push_back(keyframe);
            }

            fbxAnim.channels.push_back(fbxChannel);
        }

        model.animations.push_back(fbxAnim);

        std::cout << "  Loaded animation '" << fbxAnim.name << "' - "
                  << "Duration: " << fbxAnim.duration << " ticks, "
                  << "FPS: " << fbxAnim.ticksPerSecond << ", "
                  << "Channels: " << fbxAnim.channels.size() << std::endl;
    }
}
