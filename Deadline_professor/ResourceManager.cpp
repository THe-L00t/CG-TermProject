#include "ResourceManager.h"

ResourceManager* ResourceManager::onceInstance = nullptr;

ResourceManager::ResourceManager()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
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
            // f 1/1/1 2/2/2 3/3/3 또는 f 1//1 2//1 3//1 형식 처리
            std::vector<std::string> faceVertices;
            std::string faceVertex;
            while (ss >> faceVertex) {
                faceVertices.push_back(faceVertex);
            }

            // 정점 파싱 람다 함수
            auto parseVertex = [&](const std::string& f) -> Vertex {
                int posIdx = 0, uvIdx = 0, norIdx = 0;

                // f 형식 파싱: v, v/vt, v/vt/vn, v//vn
                size_t firstSlash = f.find('/');
                if (firstSlash == std::string::npos) {
                    // v 형식
                    posIdx = std::stoi(f);
                }
                else {
                    size_t secondSlash = f.find('/', firstSlash + 1);
                    posIdx = std::stoi(f.substr(0, firstSlash));

                    if (secondSlash != std::string::npos) {
                        // v/vt/vn 또는 v//vn 형식
                        if (secondSlash != firstSlash + 1) {
                            // v/vt/vn 형식
                            uvIdx = std::stoi(f.substr(firstSlash + 1, secondSlash - firstSlash - 1));
                        }
                        // v//vn 또는 v/vt/vn 형식의 normal 파싱
                        if (secondSlash + 1 < f.length()) {
                            norIdx = std::stoi(f.substr(secondSlash + 1));
                        }
                    }
                    else {
                        // v/vt 형식
                        uvIdx = std::stoi(f.substr(firstSlash + 1));
                    }
                }

                Vertex vertex;
                // position (필수)
                if (posIdx > 0 && posIdx <= temp_positions.size()) {
                    vertex.position = temp_positions[posIdx - 1];
                }
                // texcoord (선택)
                if (uvIdx > 0 && uvIdx <= temp_texcoords.size()) {
                    vertex.texcoord = temp_texcoords[uvIdx - 1];
                }
                else {
                    vertex.texcoord = glm::vec2(0.0f, 0.0f); // 기본값
                }
                // normal (선택)
                if (norIdx > 0 && norIdx <= temp_normals.size()) {
                    vertex.normal = temp_normals[norIdx - 1];
                }
                else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f); // 기본값
                }
                return vertex;
            };

            // 삼각형이면 바로 처리, 아니면 fan triangulation
            if (faceVertices.size() == 3) {
                // 삼각형 - 바로 처리 (최적화)
                for (const auto& fv : faceVertices) {
                    vertices.push_back(parseVertex(fv));
                    indices.push_back(vertices.size() - 1);
                }
            }
            else if (faceVertices.size() > 3) {
                // Quad 이상 - fan triangulation
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
    temp.name = name;
    temp.indexCount = indices.size();

    glGenBuffers(1, &(temp.VBO));
    glGenBuffers(1, &(temp.EBO));

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, temp.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, temp.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    //추후 셰이더 형식 정하면서 수정하기 --------------
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)0);

    // Texcoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

    // Normal
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // 정리
    glBindVertexArray(0);
    //-----------------------------------------------

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
    for (const auto& obj : dataList) {
        if (obj.name == name) {
            return &obj;
        }
    }
    return nullptr;
}

bool ResourceManager::LoadXMesh(const std::string_view& name, const std::filesystem::path& path)
{
    // 파일 열기
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs) {
        std::cerr << "f-LoadXMesh failed: " << path << std::endl;
        return false;
    }

    // 파일 크기 확인 및 전체 읽기
    size_t fileSize = ifs.tellg();
    ifs.seekg(0);

    XMeshData meshData;
    meshData.name = name;
    meshData.file_buffer.resize(fileSize);
    ifs.read(reinterpret_cast<char*>(meshData.file_buffer.data()), fileSize);
    ifs.close();

    // 헤더 검증
    const XMeshHeader* header = reinterpret_cast<const XMeshHeader*>(meshData.file_buffer.data());
    if (std::memcmp(header->magic, "XMESH\0", 6) != 0) {
        std::cerr << "Invalid XMesh file: magic mismatch" << std::endl;
        return false;
    }

    std::cout << "Loading XMesh '" << name << "' version " << header->version
              << " with " << header->chunk_count << " chunks" << std::endl;

    // 청크 테이블 읽기
    const ChunkEntry* chunkTable = reinterpret_cast<const ChunkEntry*>(
        meshData.file_buffer.data() + header->chunk_table_offset);

    // 청크 처리
    for (uint32_t i = 0; i < header->chunk_count; ++i) {
        const ChunkEntry& chunk = chunkTable[i];
        const uint8_t* payload = meshData.file_buffer.data() + chunk.chunk_offset;

        switch (chunk.chunk_type) {
        case CHUNK_VERTEX_STREAM: {
            const VertexStreamHeader* streamHeader =
                reinterpret_cast<const VertexStreamHeader*>(payload);
            const void* streamData = payload + sizeof(VertexStreamHeader);

            XMeshData::StreamInfo streamInfo;
            streamInfo.data = streamData;
            streamInfo.size = chunk.decompressed_size - sizeof(VertexStreamHeader);
            streamInfo.element_size = streamHeader->element_size;
            streamInfo.count = streamHeader->count;
            streamInfo.stream_id = streamHeader->stream_id;

            meshData.streams.push_back(streamInfo);

            std::cout << "  Stream " << streamHeader->stream_id
                      << ": " << streamHeader->count << " elements, "
                      << streamHeader->element_size << " bytes each" << std::endl;
            break;
        }

        case CHUNK_INDEX_STREAM: {
            const IndexStreamHeader* indexHeader =
                reinterpret_cast<const IndexStreamHeader*>(payload);
            meshData.index_data = payload + sizeof(IndexStreamHeader);
            meshData.index_size = chunk.decompressed_size - sizeof(IndexStreamHeader);
            meshData.index_count = indexHeader->index_count;
            meshData.index_type = (indexHeader->index_type == 16) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            std::cout << "  Indices: " << indexHeader->index_count
                      << " (" << indexHeader->index_type << " bit)" << std::endl;
            break;
        }

        case CHUNK_MESH_SECTIONS: {
            size_t sectionCount = chunk.decompressed_size / sizeof(MeshSection);
            meshData.sections.resize(sectionCount);
            std::memcpy(meshData.sections.data(), payload, chunk.decompressed_size);

            std::cout << "  Mesh sections: " << sectionCount << std::endl;
            break;
        }

        case CHUNK_SKELETON: {
            const SkeletonHeader* skelHeader =
                reinterpret_cast<const SkeletonHeader*>(payload);
            const BoneInfo* bonesData = reinterpret_cast<const BoneInfo*>(
                payload + sizeof(SkeletonHeader));

            meshData.bones.resize(skelHeader->bone_count);
            std::memcpy(meshData.bones.data(), bonesData,
                       skelHeader->bone_count * sizeof(BoneInfo));
            meshData.has_skeleton = true;

            std::cout << "  Skeleton: " << skelHeader->bone_count << " bones" << std::endl;
            break;
        }

        case CHUNK_ANIMATIONS: {
            // 애니메이션 헤더 읽기
            const AnimationHeader* animHeader =
                reinterpret_cast<const AnimationHeader*>(payload);

            AnimationClip clip;
            std::memcpy(clip.name, animHeader->name, 64);
            clip.duration = animHeader->duration;
            clip.ticks_per_second = animHeader->ticks_per_second;

            // 트랙 데이터 읽기
            const uint8_t* trackData = payload + sizeof(AnimationHeader);
            for (uint32_t t = 0; t < animHeader->track_count; ++t) {
                const TrackHeader* trackHeader =
                    reinterpret_cast<const TrackHeader*>(trackData);

                AnimationTrack track;
                track.bone_index = trackHeader->bone_index;
                track.keyframe_count = trackHeader->keyframe_count;

                // 키프레임 데이터 읽기
                const AnimationKeyframe* keyframes =
                    reinterpret_cast<const AnimationKeyframe*>(
                        trackData + sizeof(TrackHeader));

                track.keyframes.resize(trackHeader->keyframe_count);
                std::memcpy(track.keyframes.data(), keyframes,
                           trackHeader->keyframe_count * sizeof(AnimationKeyframe));

                clip.tracks.push_back(std::move(track));

                // 다음 트랙으로 이동
                trackData += sizeof(TrackHeader) +
                            trackHeader->keyframe_count * sizeof(AnimationKeyframe);
            }

            meshData.animations.push_back(std::move(clip));

            std::cout << "  Animation '" << animHeader->name << "': "
                      << animHeader->duration << "s, "
                      << animHeader->track_count << " tracks" << std::endl;
            break;
        }

        case CHUNK_META:
        case CHUNK_MATERIALS:
        case CHUNK_BOUNDING:
        case CHUNK_CUSTOM:
            // 향후 지원 예정
            std::cout << "  Chunk type 0x" << std::hex << chunk.chunk_type
                      << std::dec << " (not implemented yet)" << std::endl;
            break;

        default:
            std::cerr << "  Unknown chunk type: 0x" << std::hex << chunk.chunk_type << std::dec << std::endl;
            break;
        }
    }

    // GPU 업로드
    glBindVertexArray(VAO);

    // VBO 생성 및 업로드 (스트림별)
    meshData.vbos.resize(meshData.streams.size());
    glGenBuffers(static_cast<GLsizei>(meshData.vbos.size()), meshData.vbos.data());

    for (size_t s = 0; s < meshData.streams.size(); ++s) {
        const auto& stream = meshData.streams[s];
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vbos[s]);
        glBufferData(GL_ARRAY_BUFFER, stream.size, stream.data, GL_STATIC_DRAW);

        // 스트림 ID에 따라 vertex attribute 설정
        // stream 0: position (vec3)
        // stream 1: normal (vec3)
        // stream 2: uv0 (vec2)
        GLuint attribIndex = stream.stream_id;
        GLint componentCount = stream.element_size / sizeof(float);

        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex, componentCount, GL_FLOAT, GL_FALSE,
                              stream.element_size, (void*)0);
    }

    // EBO 생성 및 업로드
    if (meshData.index_data) {
        glGenBuffers(1, &meshData.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData.index_size,
                     meshData.index_data, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    std::cout << "XMesh '" << name << "' loaded successfully with "
              << meshData.index_count << " indices" << std::endl;

    xmeshList.push_back(std::move(meshData));
    return true;
}

const XMeshData* ResourceManager::GetXMeshData(const std::string_view& name) const
{
    for (const auto& mesh : xmeshList) {
        if (mesh.name == name) {
            return &mesh;
        }
    }
    return nullptr;
}
