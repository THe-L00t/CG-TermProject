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
            // f 1/1/1 2/2/2 3/3/3 형식 처리
            std::string v1, v2, v3;
            ss >> v1 >> v2 >> v3;
            std::vector<std::string> face = { v1, v2, v3 };

            for (auto& f : face) {
                int posIdx, uvIdx, norIdx;
                sscanf(f.c_str(), "%d/%d/%d", &posIdx, &uvIdx, &norIdx);

                Vertex vertex;
                vertex.position = temp_positions[posIdx - 1];
                vertex.texcoord = temp_texcoords[uvIdx - 1];
                vertex.normal = temp_normals[norIdx - 1];

                vertices.push_back(vertex);
                indices.push_back(vertices.size()-1);
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
