#pragma once
#include "TotalHeader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
};

struct ObjData {
	std::string_view name;
	GLuint VBO{};
	GLuint EBO{};
};

class ResourceManager
{
public:
	ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;


	void Active();
	void Deactive();

	bool LoadObj(const std::string_view&, const std::filesystem::path&);

private:
	static ResourceManager* onceInstance;
	GLuint VAO{};
	std::vector<ObjData> dataList;
};

