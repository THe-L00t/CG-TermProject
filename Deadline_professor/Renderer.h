#pragma once
#include "TotalHeader.h"
#include "Shader.h"
#include "ResourceManager.h"

class Camera;

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
	void SetCamera(Camera* cam);

	void OnWindowResize(int, int);

	//콜백 함수
	std::function<void(GLvoid)> onDrawScene;
	static void DrawScene(GLvoid);

	// 셰이더 관리
	bool LoadShader(const std::string&, const std::filesystem::path&, const std::filesystem::path&);
	Shader* GetShader(const std::string&);
	void RenderTestCube();

	// XMesh 렌더링
	void RenderXMesh(const std::string_view& meshName, const glm::mat4& modelMatrix = glm::mat4(1.0f));
	void RenderXMeshSection(const std::string_view& meshName, size_t sectionIndex, const glm::mat4& modelMatrix = glm::mat4(1.0f));

	// 스켈레톤 애니메이션 렌더링
	void RenderAnimatedMesh(const std::string_view& meshName, const std::vector<glm::mat4>& boneTransforms, const glm::mat4& modelMatrix = glm::mat4(1.0f));

private:
	static Renderer* activeInstance;

	std::unordered_map<std::string, Shader> shaders;
	ResourceManager* resourceManager;
	Camera* camera;

	// 테스트용 변수
	GLuint testVAO{};
	GLuint testVBO{};
	GLuint testEBO{};
	size_t testIndexCount{};
};

