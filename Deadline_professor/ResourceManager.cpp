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
