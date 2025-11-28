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
        std::cerr << "f-LoadXMesh failed: Cannot open file " << path << std::endl;
        return false;
    }

    // 파일 크기 확인 및 전체 읽기
    size_t fileSize = ifs.tellg();
    if (fileSize < sizeof(XMeshHeader)) {
        std::cerr << "f-LoadXMesh failed: File too small (" << fileSize << " bytes)" << std::endl;
        return false;
    }

    ifs.seekg(0);

    XMeshData meshData;
    meshData.name = name;
    meshData.file_buffer.resize(fileSize);
    ifs.read(reinterpret_cast<char*>(meshData.file_buffer.data()), fileSize);
    ifs.close();

    std::cout << "File loaded: " << fileSize << " bytes" << std::endl;

    // 헤더 검증
    const XMeshHeader* header = reinterpret_cast<const XMeshHeader*>(meshData.file_buffer.data());

    // Magic number 출력 (디버깅용)
    std::cout << "Magic: ";
    for (int i = 0; i < 6; i++) {
        std::cout << std::hex << (int)(unsigned char)header->magic[i] << " ";
    }
    std::cout << std::dec << std::endl;

    // Magic number 검증 (XMESH + null 또는 XMESH + 0x1A 허용)
    bool validMagic = (std::memcmp(header->magic, "XMESH\0", 6) == 0) ||
                      (std::memcmp(header->magic, "XMESH\x1A", 6) == 0) ||
                      (std::memcmp(header->magic, "XMESH", 5) == 0);

    if (!validMagic) {
        std::cerr << "Invalid XMesh file: magic mismatch (expected XMESH)" << std::endl;
        return false;
    }

    std::cout << "Loading XMesh '" << name << "' version " << header->version
              << " with " << header->chunk_count << " chunks" << std::endl;
    std::cout << "Chunk table offset: " << header->chunk_table_offset << std::endl;
    std::cout << "File size: " << header->file_size << std::endl;

    // 청크 테이블 오프셋 검증
    if (header->chunk_table_offset >= fileSize) {
        std::cerr << "Invalid chunk_table_offset: " << header->chunk_table_offset
                  << " (file size: " << fileSize << ")" << std::endl;
        return false;
    }

    // 청크 테이블 읽기
    const ChunkEntry* chunkTable = reinterpret_cast<const ChunkEntry*>(
        meshData.file_buffer.data() + header->chunk_table_offset);

    // 청크 처리
    for (uint32_t i = 0; i < header->chunk_count; ++i) {
        const ChunkEntry& chunk = chunkTable[i];

        std::cout << "Processing chunk " << i << ": type=0x" << std::hex << chunk.chunk_type
                  << std::dec << ", offset=" << chunk.chunk_offset
                  << ", size=" << chunk.decompressed_size << std::endl;

        // 청크 오프셋 검증
        if (chunk.chunk_offset >= fileSize) {
            std::cerr << "Invalid chunk offset: " << chunk.chunk_offset << std::endl;
            continue;
        }

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
            const BoneEntry* bonesData = reinterpret_cast<const BoneEntry*>(
                payload + sizeof(SkeletonHeader));

            // 이름 테이블 위치
            const char* nameTable = nullptr;
            if (skelHeader->name_table_offset > 0) {
                nameTable = reinterpret_cast<const char*>(
                    payload + skelHeader->name_table_offset);
            }

            meshData.bones.resize(skelHeader->bone_count);
            for (uint32_t b = 0; b < skelHeader->bone_count; ++b) {
                const BoneEntry& entry = bonesData[b];
                BoneInfo& bone = meshData.bones[b];

                // 이름 읽기
                if (nameTable && entry.name_offset != static_cast<uint32_t>(-1)) {
                    bone.name = std::string(nameTable + entry.name_offset);
                    meshData.bone_name_to_index[bone.name] = b;
                } else {
                    bone.name = "bone_" + std::to_string(b);
                }

                bone.parent_index = entry.parent_index;
                bone.flags = entry.flags;

                // 행렬 복사 (column-major)
                std::memcpy(&bone.local_bind[0][0], entry.local_bind, 16 * sizeof(float));
                std::memcpy(&bone.inv_bind[0][0], entry.inv_bind, 16 * sizeof(float));
            }

            meshData.has_skeleton = true;
            std::cout << "  Skeleton: " << skelHeader->bone_count << " bones" << std::endl;
            break;
        }

        case CHUNK_SKINNING: {
            // 스키닝 속성 (bone indices, weights)은 vertex stream으로 처리됨
            meshData.has_skinning = true;
            std::cout << "  Skinning data loaded" << std::endl;
            break;
        }

        case CHUNK_ANIMATION_INDEX: {
            const AnimIndexHeader* animIndexHeader =
                reinterpret_cast<const AnimIndexHeader*>(payload);

            const uint8_t* clipData = payload + sizeof(AnimIndexHeader);

            for (uint32_t c = 0; c < animIndexHeader->clip_count; ++c) {
                const AnimClipEntry* clipEntry =
                    reinterpret_cast<const AnimClipEntry*>(clipData);

                AnimationClip clip;

                // 이름 읽기 (간단한 구현: 클립 엔트리 바로 뒤에 이름 테이블이 있다고 가정)
                const char* clipName = reinterpret_cast<const char*>(
                    clipData + clipEntry->name_offset);
                clip.name = std::string(clipName);
                clip.duration = clipEntry->duration;
                clip.sample_rate = clipEntry->sample_rate;

                // 트랙 테이블 읽기
                const TrackTableEntry* trackTable =
                    reinterpret_cast<const TrackTableEntry*>(
                        clipData + clipEntry->track_table_offset);

                clip.tracks.resize(clipEntry->num_bone_tracks);

                std::cout << "  Animation '" << clip.name << "': "
                          << clip.duration << "s, "
                          << clipEntry->num_bone_tracks << " tracks" << std::endl;

                meshData.animations.push_back(std::move(clip));
                meshData.anim_name_to_index[clip.name] = c;

                // 다음 클립으로 이동 (간단한 구현)
                clipData += sizeof(AnimClipEntry) +
                           clipEntry->num_bone_tracks * sizeof(TrackTableEntry) +
                           256; // 이름 버퍼 크기 (임시)
            }
            break;
        }

        case CHUNK_ANIM_TRACK: {
            // 애니메이션 트랙은 ANIMATION_INDEX에서 참조됨
            // 실제 디코딩은 AnimationPlayer에서 수행
            std::cout << "  Anim track chunk (will be decoded on demand)" << std::endl;
            break;
        }

        case CHUNK_META:
        case CHUNK_MATERIALS:
        case CHUNK_BOUNDING:
        case CHUNK_LOD:
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
    std::cout << "Starting GPU upload..." << std::endl;

    if (meshData.streams.empty()) {
        std::cerr << "Warning: No vertex streams found" << std::endl;
    }

    if (!meshData.index_data) {
        std::cerr << "Warning: No index data found" << std::endl;
    }

    glBindVertexArray(VAO);

    // VBO 생성 및 업로드 (스트림별)
    if (!meshData.streams.empty()) {
        meshData.vbos.resize(meshData.streams.size());
        glGenBuffers(static_cast<GLsizei>(meshData.vbos.size()), meshData.vbos.data());

        for (size_t s = 0; s < meshData.streams.size(); ++s) {
            const auto& stream = meshData.streams[s];

            std::cout << "Uploading stream " << s << " (ID=" << stream.stream_id
                      << ", size=" << stream.size << ")" << std::endl;

            if (stream.size == 0 || stream.data == nullptr) {
                std::cerr << "Warning: Stream " << s << " has invalid data" << std::endl;
                continue;
            }

            glBindBuffer(GL_ARRAY_BUFFER, meshData.vbos[s]);
            glBufferData(GL_ARRAY_BUFFER, stream.size, stream.data, GL_STATIC_DRAW);

            // 스트림 ID에 따라 vertex attribute 설정
            GLuint attribIndex = stream.stream_id;
            GLint componentCount = stream.element_size / sizeof(float);

            if (componentCount > 0 && componentCount <= 4) {
                glEnableVertexAttribArray(attribIndex);
                glVertexAttribPointer(attribIndex, componentCount, GL_FLOAT, GL_FALSE,
                                      stream.element_size, (void*)0);
            } else {
                std::cerr << "Warning: Invalid component count " << componentCount
                          << " for stream " << s << std::endl;
            }
        }
    }

    // EBO 생성 및 업로드
    if (meshData.index_data && meshData.index_size > 0) {
        std::cout << "Uploading index buffer (size=" << meshData.index_size << ")" << std::endl;
        glGenBuffers(1, &meshData.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData.index_size,
                     meshData.index_data, GL_STATIC_DRAW);
    }

    glBindVertexArray(0);

    std::cout << "GPU upload complete" << std::endl;

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

bool ResourceManager::DecodeAnimationTrack(const XMeshData* mesh, uint32_t clipIndex, uint32_t trackIndex, AnimationTrack& outTrack)
{
    if (!mesh || clipIndex >= mesh->animations.size()) {
        std::cerr << "Invalid clip index" << std::endl;
        return false;
    }

    const AnimationClip& clip = mesh->animations[clipIndex];
    if (trackIndex >= clip.tracks.size()) {
        std::cerr << "Invalid track index" << std::endl;
        return false;
    }

    // 간단한 구현: 이미 디코딩된 트랙 반환
    // 실제로는 CHUNK_ANIM_TRACK에서 압축 해제 필요
    outTrack = clip.tracks[trackIndex];

    return true;
}
