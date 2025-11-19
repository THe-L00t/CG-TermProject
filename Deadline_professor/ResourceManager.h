#pragma once
#include "TotalHeader.h"

struct Vertex {
	glm::vec3 pos;
	glm::vec2 texcoord;
	glm::vec3 normal;
};

struct ObjData {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class ResourceManager
{
public:

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;


	void Active();
	void Deactive();

private:
	static ResourceManager* onceInstance;

};

