#pragma once
#include "TotalHeader.h"
#include "Shader.h"
#include "ResourceManager.h"
class Renderer
{
public:
	Renderer(ResourceManager* resMgr);
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	void Init();
	void Active();
	void Deactive();

	void OnWindowResize(int, int);

	//콜백 함수
	std::function<void(GLvoid)> onDrawScene;
	static void DrawScene(GLvoid);

	// 셰이더 관리
	bool LoadShader(const std::string&, const std::filesystem::path&, const std::filesystem::path&);
	Shader* GetShader(const std::string&);
	void RenderTestCube();

private:
	static Renderer* activeInstance;

	std::unordered_map<std::string, Shader> shaders;
	ResourceManager* resourceManager;

	// 테스트용 변수
	GLuint testVAO{};
	GLuint testVBO{};
	GLuint testEBO{};
	size_t testIndexCount{};
};

