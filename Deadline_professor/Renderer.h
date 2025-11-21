#pragma once
#include "TotalHeader.h"
#include "Shader.h"
#include "ResourceManager.h"
class Renderer
{
public:
	Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void Init();

	void SetViewport(int, int, int, int);
	void OnWindowResize(int, int);

private:
	std::unordered_map<std::string, Shader> shaders;
	ResourceManager reManager{};
};

