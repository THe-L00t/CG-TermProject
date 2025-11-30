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
            streamInfo.stride = streamHeader->stride;  // ✅ stride 저장

            meshData.streams.push_back(streamInfo);

            // Position stream (stream_id=0)이고 int16x3 포맷이면 quantization 메타데이터 계산
            if (streamHeader->stream_id == 0 && streamHeader->element_size == 6) {
                const int16_t* posData = reinterpret_cast<const int16_t*>(streamData);

                // ✅ 올바른 quantization 메타데이터 계산
                // 실제 메시 범위 분석
                glm::vec3 minPos(FLT_MAX);
                glm::vec3 maxPos(-FLT_MAX);

                for (uint32_t v = 0; v < streamHeader->count; ++v) {
                    glm::vec3 pos(
                        static_cast<float>(posData[v * 3 + 0]),
                        static_cast<float>(posData[v * 3 + 1]),
                        static_cast<float>(posData[v * 3 + 2])
                    );
                    minPos = glm::min(minPos, pos);
                    maxPos = glm::max(maxPos, pos);
                }

                // ✅ 메시를 원점 중심으로, 정규화 범위 [-1, 1]로 디코딩
                glm::vec3 center = (minPos + maxPos) * 0.5f;
                glm::vec3 extent = (maxPos - minPos) * 0.5f;
                float maxExtent = glm::max(glm::max(extent.x, extent.y), extent.z);

                meshData.pos_offset = center;
                meshData.pos_scale = 1.0f / maxExtent;  // 정규화 [-1, 1]

                std::cout << "  [Quantization Metadata - COMPUTED]:" << std::endl;
                std::cout << "    Raw int16 range: Min(" << minPos.x << ", " << minPos.y << ", " << minPos.z << ")"
                          << " Max(" << maxPos.x << ", " << maxPos.y << ", " << maxPos.z << ")" << std::endl;
                std::cout << "    Center: (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
                std::cout << "    Max extent: " << maxExtent << std::endl;
                std::cout << "    Offset: (" << meshData.pos_offset.x << ", " << meshData.pos_offset.y << ", " << meshData.pos_offset.z << ")" << std::endl;
                std::cout << "    Scale: " << meshData.pos_scale << std::endl;

                // 검증: 첫 번째 정점 디코딩
                glm::vec3 firstRaw(posData[0], posData[1], posData[2]);
                glm::vec3 decoded = (firstRaw - meshData.pos_offset) * meshData.pos_scale;
                std::cout << "    First vertex: raw(" << firstRaw.x << ", " << firstRaw.y << ", " << firstRaw.z << ")"
                          << " → decoded(" << decoded.x << ", " << decoded.y << ", " << decoded.z << ")" << std::endl;

                // 예상 world space 범위 (정규화된 좌표)
                glm::vec3 worldMin = (minPos - center) * meshData.pos_scale;
                glm::vec3 worldMax = (maxPos - center) * meshData.pos_scale;
                std::cout << "    Normalized range: Min(" << worldMin.x << ", " << worldMin.y << ", " << worldMin.z << ")"
                          << " Max(" << worldMax.x << ", " << worldMax.y << ", " << worldMax.z << ")" << std::endl;
            }

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
            std::cout << "    sizeof(IndexStreamHeader): " << sizeof(IndexStreamHeader) << " bytes" << std::endl;

            // ✅ CRITICAL: header_size 필드를 사용해서 정확한 오프셋 계산
            uint32_t actualHeaderSize = indexHeader->header_size;
            if (actualHeaderSize != sizeof(IndexStreamHeader)) {
                std::cout << "    WARNING: sizeof() mismatch! Using header_size from file: " << actualHeaderSize << std::endl;
            }

            meshData.index_data = payload + actualHeaderSize;  // ✅ 파일의 header_size 사용
            meshData.index_size = chunk.decompressed_size - actualHeaderSize;
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
            if (stream.stream_id == 0 && stream.element_size == 6) {
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

            // Bone weights 검증
            if (stream.stream_id == 6 && stream.element_size == 4) {
                const uint8_t* weightData = reinterpret_cast<const uint8_t*>(stream.data);
                std::cout << "  [DEBUG] First 3 bone weights (u8x4, normalized):" << std::endl;
                for (int v = 0; v < 3 && v < static_cast<int>(stream.count); ++v) {
                    std::cout << "    v" << v << ": ("
                              << (int)weightData[v*4+0] << ", "
                              << (int)weightData[v*4+1] << ", "
                              << (int)weightData[v*4+2] << ", "
                              << (int)weightData[v*4+3] << ")" << std::endl;
                }
            }

            // Bone indices 검증
            if (stream.stream_id == 7 && stream.element_size == 4) {
                const uint8_t* indexData = reinterpret_cast<const uint8_t*>(stream.data);
                std::cout << "  [DEBUG] First 3 bone indices (u8x4):" << std::endl;
                for (int v = 0; v < 3 && v < static_cast<int>(stream.count); ++v) {
                    std::cout << "    v" << v << ": ("
                              << (int)indexData[v*4+0] << ", "
                              << (int)indexData[v*4+1] << ", "
                              << (int)indexData[v*4+2] << ", "
                              << (int)indexData[v*4+3] << ")" << std::endl;
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
            case 0: // position (int16x3, normalized=false → ivec3)
                attribIndex = 0;
                if (stream.element_size == 12) {
                    componentCount = 3;
                    dataType = GL_FLOAT;
                    normalized = GL_FALSE;
                } else if (stream.element_size == 6) {
                    componentCount = 3;
                    dataType = GL_SHORT;
                    normalized = GL_FALSE; // ✅ CRITICAL: raw int16 → ivec3
                }
                break;

            case 1: // normal (int16x3, normalized=true → vec3)
                attribIndex = 1;
                if (stream.element_size == 12) {
                    componentCount = 3;
                    dataType = GL_FLOAT;
                    normalized = GL_FALSE;
                } else if (stream.element_size == 6) {
                    componentCount = 3;
                    dataType = GL_SHORT;
                    normalized = GL_TRUE; // ✅ normalized → [-1,1] vec3
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
                normalized = GL_FALSE;  // ✅ CRITICAL: integer indices, no normalization
                break;

            default:
                std::cerr << "Warning: Unknown stream ID " << stream.stream_id << std::endl;
                continue;
            }

            if (componentCount > 0 && componentCount <= 4) {
                glEnableVertexAttribArray(attribIndex);

                // ✅ XMesh 명세에 따른 정확한 attribute 설정
                // Position (stream 0, int16, normalized=false) → glVertexAttribIPointer → ivec3
                // Normal (stream 1, int16, normalized=true) → glVertexAttribPointer → vec3
                // UV (stream 2/3, half) → glVertexAttribPointer → vec2
                // BoneWeights (stream 6, u8, normalized=true) → glVertexAttribPointer → vec4
                // BoneIndices (stream 7, u8, normalized=false) → glVertexAttribIPointer → ivec4

                bool useIntegerAttrib = false;

                // Position: int16 raw → ivec3
                if (stream.stream_id == 0 && dataType == GL_SHORT && normalized == GL_FALSE) {
                    useIntegerAttrib = true;
                }
                // BoneIndices: u8 raw → ivec4
                else if (stream.stream_id == 7 && dataType == GL_UNSIGNED_BYTE && normalized == GL_FALSE) {
                    useIntegerAttrib = true;
                }

                // ✅ XMesh 스펙: stride가 0이면 tightly packed (element_size 사용)
                GLsizei actualStride = (stream.stride == 0) ? stream.element_size : stream.stride;

                if (useIntegerAttrib) {
                    // ✅ Integer attribute (ivec*)
                    glVertexAttribIPointer(attribIndex, componentCount, dataType,
                                          actualStride, (void*)0);
                    std::cout << "  -> Enabled INTEGER attrib " << attribIndex
                              << " (stream_id=" << stream.stream_id << ")"
                              << " with " << componentCount << " components"
                              << " (type=" << (dataType == GL_SHORT ? "SHORT" :
                                               dataType == GL_UNSIGNED_BYTE ? "UBYTE" : "OTHER")
                              << ", stride=" << actualStride
                              << ") → GLSL: " << (componentCount == 3 ? "ivec3" : "ivec4")
                              << std::endl;
                } else {
                    // ✅ Float attribute (vec*)
                    glVertexAttribPointer(attribIndex, componentCount, dataType, normalized,
                                          actualStride, (void*)0);
                    std::cout << "  -> Enabled FLOAT attrib " << attribIndex
                              << " (stream_id=" << stream.stream_id << ")"
                              << " with " << componentCount << " components"
                              << " (type=" << (dataType == GL_FLOAT ? "FLOAT" :
                                               dataType == GL_SHORT ? "SHORT" :
                                               dataType == GL_UNSIGNED_BYTE ? "UBYTE" :
                                               dataType == GL_HALF_FLOAT ? "HALF_FLOAT" : "OTHER")
                              << ", normalized=" << (normalized ? "true" : "false")
                              << ", stride=" << actualStride << ")"
                              << " → GLSL: vec" << componentCount
                              << std::endl;
                }
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

    // 스키닝 데이터가 없는 스태틱 메시를 위한 더미 bone attributes 추가
    bool hasBoneWeights = false;
    bool hasBoneIndices = false;
    for (const auto& stream : meshData.streams) {
        if (stream.stream_id == 6) hasBoneWeights = true;
        if (stream.stream_id == 7) hasBoneIndices = true;
    }

    if (!hasBoneWeights || !hasBoneIndices) {
        // 정점 개수 찾기
        uint32_t vertexCount = 0;
        for (const auto& stream : meshData.streams) {
            if (stream.stream_id == 0) {
                vertexCount = stream.count;
                break;
            }
        }

        if (vertexCount > 0) {
            std::cout << "Adding dummy bone attributes for static mesh (" << vertexCount << " vertices)" << std::endl;

            if (!hasBoneWeights) {
                // 더미 bone weights (1.0, 0, 0, 0) - 첫 번째 본에만 100% 가중치
                std::vector<float> dummyWeights(vertexCount * 4);
                for (uint32_t i = 0; i < vertexCount; ++i) {
                    dummyWeights[i * 4 + 0] = 1.0f;
                    dummyWeights[i * 4 + 1] = 0.0f;
                    dummyWeights[i * 4 + 2] = 0.0f;
                    dummyWeights[i * 4 + 3] = 0.0f;
                }

                GLuint dummyWeightsVBO;
                glGenBuffers(1, &dummyWeightsVBO);
                glBindBuffer(GL_ARRAY_BUFFER, dummyWeightsVBO);
                glBufferData(GL_ARRAY_BUFFER, dummyWeights.size() * sizeof(float),
                            dummyWeights.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
                meshData.vbos.push_back(dummyWeightsVBO);
            }

            if (!hasBoneIndices) {
                // 더미 bone indices (0, 0, 0, 0)
                std::vector<uint8_t> dummyIndices(vertexCount * 4, 0);

                GLuint dummyIndicesVBO;
                glGenBuffers(1, &dummyIndicesVBO);
                glBindBuffer(GL_ARRAY_BUFFER, dummyIndicesVBO);
                glBufferData(GL_ARRAY_BUFFER, dummyIndices.size(),
                            dummyIndices.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(6);
                glVertexAttribIPointer(6, 4, GL_UNSIGNED_BYTE, 0, (void*)0);
                meshData.vbos.push_back(dummyIndicesVBO);
            }
        }
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
