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
    temp.name = std::string(name);
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
    std::string nameStr(name);
    for (const auto& obj : dataList) {
        if (obj.name == nameStr) {
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
    meshData.name = std::string(name);
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

            // 헤더 검증
            std::cout << "  [VERTEX_STREAM Header]:" << std::endl;
            std::cout << "    stream_id: " << streamHeader->stream_id << std::endl;
            std::cout << "    element_size: " << streamHeader->element_size << std::endl;
            std::cout << "    count: " << streamHeader->count << std::endl;
            std::cout << "    stride: " << streamHeader->stride << std::endl;
            std::cout << "    format_flags: 0x" << std::hex << streamHeader->format_flags << std::dec << std::endl;
            std::cout << "    chunk size: " << chunk.decompressed_size << std::endl;
            std::cout << "    header size: " << sizeof(VertexStreamHeader) << std::endl;
            std::cout << "    expected data size: " << (streamHeader->count * streamHeader->element_size) << std::endl;

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

            // v2.2 스펙 검증
            if (indexHeader->header_size != 32) {
                std::cerr << "ERROR: Invalid INDEX_STREAM header_size: " << indexHeader->header_size
                          << " (expected 32)" << std::endl;
                break;
            }

            std::cout << "  [DEBUG] IndexStreamHeader (v2.2):" << std::endl;
            std::cout << "    header_size: " << indexHeader->header_size << std::endl;
            std::cout << "    index_count: " << indexHeader->index_count << std::endl;
            std::cout << "    index_type: " << indexHeader->index_type
                      << " (" << (indexHeader->index_type == 0 ? "uint32" : "uint16") << ")" << std::endl;
            std::cout << "    primitive_type: " << indexHeader->primitive_type << std::endl;

            meshData.index_data = payload + sizeof(IndexStreamHeader);
            meshData.index_size = chunk.decompressed_size - sizeof(IndexStreamHeader);
            meshData.index_count = indexHeader->index_count;

            // index_type: 0=uint32 (4 bytes), 1=uint16 (2 bytes)
            meshData.index_type = (indexHeader->index_type == 1) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;

            size_t bytesPerIndex = (indexHeader->index_type == 1) ? 2 : 4;
            size_t expectedSize = meshData.index_count * bytesPerIndex;

            std::cout << "  Indices: " << meshData.index_count
                      << " (" << bytesPerIndex << " bytes per index)"
                      << ", payload size: " << meshData.index_size
                      << " bytes (expected: " << expectedSize << ")" << std::endl;

            // 크기 검증
            if (meshData.index_size != expectedSize) {
                std::cerr << "WARNING: Index payload size mismatch!" << std::endl;
            }
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

            // 이름 테이블 위치 및 범위
            const char* nameTable = nullptr;
            size_t nameTableSize = 0;
            if (skelHeader->name_table_offset > 0 &&
                skelHeader->name_table_offset < chunk.decompressed_size) {
                nameTable = reinterpret_cast<const char*>(
                    payload + skelHeader->name_table_offset);
                nameTableSize = chunk.decompressed_size - skelHeader->name_table_offset;
            }

            meshData.bones.resize(skelHeader->bone_count);
            for (uint32_t b = 0; b < skelHeader->bone_count; ++b) {
                const BoneEntry& entry = bonesData[b];
                BoneInfo& bone = meshData.bones[b];

                // 이름 읽기 (안전하게 처리)
                if (nameTable &&
                    entry.name_offset != static_cast<uint32_t>(-1) &&
                    entry.name_offset < nameTableSize) {
                    const char* namePtr = nameTable + entry.name_offset;
                    // null-terminated 문자열 길이를 안전하게 계산
                    size_t maxLen = nameTableSize - entry.name_offset;
                    size_t nameLen = strnlen(namePtr, maxLen);
                    bone.name = std::string(namePtr, nameLen);
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
            size_t remainingSize = chunk.decompressed_size - sizeof(AnimIndexHeader);

            for (uint32_t c = 0; c < animIndexHeader->clip_count; ++c) {
                const AnimClipEntry* clipEntry =
                    reinterpret_cast<const AnimClipEntry*>(clipData);

                AnimationClip clip;

                // 이름 읽기 (안전하게 처리)
                if (clipEntry->name_offset < remainingSize) {
                    const char* clipName = reinterpret_cast<const char*>(
                        clipData + clipEntry->name_offset);
                    size_t maxLen = remainingSize - clipEntry->name_offset;
                    size_t nameLen = strnlen(clipName, maxLen);
                    clip.name = std::string(clipName, nameLen);
                } else {
                    clip.name = "anim_" + std::to_string(c);
                }
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

    // GPU 업로드 - 각 XMesh는 자체 VAO를 가짐
    std::cout << "Starting GPU upload..." << std::endl;

    if (meshData.streams.empty()) {
        std::cerr << "Warning: No vertex streams found" << std::endl;
    }

    if (!meshData.index_data) {
        std::cerr << "Warning: No index data found" << std::endl;
    }

    // XMesh 전용 VAO 생성
    glGenVertexArrays(1, &meshData.vao);
    glBindVertexArray(meshData.vao);

    // VBO 생성 및 업로드 (스트림별)
    if (!meshData.streams.empty()) {
        meshData.vbos.resize(meshData.streams.size());
        glGenBuffers(static_cast<GLsizei>(meshData.vbos.size()), meshData.vbos.data());

        for (size_t s = 0; s < meshData.streams.size(); ++s) {
            const auto& stream = meshData.streams[s];

            std::cout << "Uploading stream " << s << " (ID=" << stream.stream_id
                      << ", size=" << stream.size
                      << ", count=" << stream.count
                      << ", element_size=" << stream.element_size << ")" << std::endl;

            if (stream.size == 0 || stream.data == nullptr) {
                std::cerr << "Warning: Stream " << s << " has invalid data" << std::endl;
                continue;
            }

            glBindBuffer(GL_ARRAY_BUFFER, meshData.vbos[s]);
            glBufferData(GL_ARRAY_BUFFER, stream.size, stream.data, GL_STATIC_DRAW);

            // 첫 몇 개 정점 데이터 출력 (디버깅)
            if (s == 0 && stream.stream_id == 0 && stream.element_size == 6) {
                // position stream (int16x3)
                const int16_t* posData = reinterpret_cast<const int16_t*>(stream.data);
                std::cout << "  [DEBUG] First 3 position vertices (int16x3):" << std::endl;
                for (int v = 0; v < 3 && v * 3 < static_cast<int>(stream.count); ++v) {
                    std::cout << "    v" << v << ": ("
                              << posData[v*3+0] << ", "
                              << posData[v*3+1] << ", "
                              << posData[v*3+2] << ")" << std::endl;
                }
            }

            // XMESH 스펙에 따른 스트림 ID → attribute location 매핑
            // stream_id    attribute_location    format
            // 0            0                     pos (float3 or int16x3)
            // 1            1                     normal (float3 or int16x3)
            // 2            2                     tangent (float4 or int16x4)
            // 3            3                     uv (float2 or half2)
            // 4            4                     color (u8x4 normalized)
            // 6            5                     weights (u8x4 or half4 normalized)
            // 7            6                     bone indices (u8x4)

            GLuint attribIndex = 0;
            GLint componentCount = 0;
            GLenum dataType = GL_FLOAT;
            GLboolean normalized = GL_FALSE;

            switch (stream.stream_id) {
            case 0: // position
                attribIndex = 0;
                if (stream.element_size == 12) {
                    componentCount = 3;
                    dataType = GL_FLOAT;
                } else if (stream.element_size == 6) {
                    componentCount = 3;
                    dataType = GL_SHORT;
                    normalized = GL_FALSE; // raw int16, 셰이더에서 변환
                }
                break;

            case 1: // normal
                attribIndex = 1;
                if (stream.element_size == 12) {
                    componentCount = 3;
                    dataType = GL_FLOAT;
                } else if (stream.element_size == 6) {
                    componentCount = 3;
                    dataType = GL_SHORT;
                    normalized = GL_TRUE; // 노말은 정규화
                }
                break;

            case 2: // tangent OR uv (파일에 따라 다름 - element_size로 구분)
                if (stream.element_size == 16) {
                    // tangent (float4)
                    attribIndex = 2;
                    componentCount = 4;
                    dataType = GL_FLOAT;
                } else if (stream.element_size == 8) {
                    // tangent (int16x4) or uv (float2)
                    if (stream.count > 100000) {
                        // 많은 버텍스 → UV 추측
                        attribIndex = 3;
                        componentCount = 2;
                        dataType = GL_FLOAT;
                    } else {
                        attribIndex = 2;
                        componentCount = 4;
                        dataType = GL_SHORT;
                        normalized = GL_TRUE;
                    }
                } else if (stream.element_size == 4) {
                    // uv (half2)
                    attribIndex = 3;
                    componentCount = 2;
                    dataType = GL_HALF_FLOAT;
                }
                break;

            case 3: // uv
                attribIndex = 3;
                if (stream.element_size == 8) {
                    componentCount = 2;
                    dataType = GL_FLOAT;
                } else if (stream.element_size == 4) {
                    componentCount = 2;
                    dataType = GL_HALF_FLOAT;
                }
                break;

            case 4: // color
                attribIndex = 4;
                componentCount = 4;
                dataType = GL_UNSIGNED_BYTE;
                normalized = GL_TRUE;
                break;

            case 6: // bone weights
                attribIndex = 5;
                if (stream.element_size == 16) {
                    componentCount = 4;
                    dataType = GL_FLOAT;
                } else if (stream.element_size == 4) {
                    componentCount = 4;
                    dataType = GL_UNSIGNED_BYTE;
                    normalized = GL_TRUE;
                }
                break;

            case 7: // bone indices
                attribIndex = 6;
                componentCount = 4;
                dataType = GL_UNSIGNED_BYTE;
                break;

            default:
                std::cerr << "Warning: Unknown stream ID " << stream.stream_id << std::endl;
                continue;
            }

            if (componentCount > 0 && componentCount <= 4) {
                glEnableVertexAttribArray(attribIndex);
                glVertexAttribPointer(attribIndex, componentCount, dataType, normalized,
                                      stream.element_size, (void*)0);
                std::cout << "  -> Enabled attrib " << attribIndex
                          << " with " << componentCount << " components"
                          << " (type=" << (dataType == GL_FLOAT ? "FLOAT" :
                                           dataType == GL_SHORT ? "SHORT" :
                                           dataType == GL_HALF_FLOAT ? "HALF_FLOAT" : "OTHER")
                          << ", normalized=" << (normalized ? "true" : "false") << ")"
                          << std::endl;
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
    std::string nameStr(name);
    for (const auto& mesh : xmeshList) {
        if (mesh.name == nameStr) {
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
