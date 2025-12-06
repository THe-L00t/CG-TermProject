#pragma once
#include "TotalHeader.h"
#include "Shader.h"
#include "ResourceManager.h"

class Camera;
class Light;

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
	void SetLight(Light* l);

	void OnWindowResize(int, int);

	//콜백 함수
	std::function<void(GLvoid)> onDrawScene;
	static void DrawScene(GLvoid);

	// 셰이더 관리
	bool LoadShader(const std::string&, const std::filesystem::path&, const std::filesystem::path&);
	Shader* GetShader(const std::string&);

	// OBJ 모델 렌더링
	void RenderObj(const std::string_view& objName, const glm::mat4& modelMatrix = glm::mat4(1.0f));
	void RenderObj(const std::string_view& objName, const glm::mat4& modelMatrix, const glm::vec3& color);
	void RenderObjWithTexture(const std::string_view& objName, const std::string_view& textureName, const glm::mat4& modelMatrix = glm::mat4(1.0f));
	void RenderObjWithTextureTiled(const std::string_view& objName, const std::string_view& textureName, const glm::mat4& modelMatrix, const glm::vec2& tiling = glm::vec2(1.0f));

	// FBX 모델 렌더링 (텍스처 포함)
	void RenderFBX(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix);
	void RenderFBXAnimated(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix, const std::vector<glm::mat4>& boneTransforms);

private:
	static Renderer* activeInstance;

	std::unordered_map<std::string, Shader> shaders;
	ResourceManager* resourceManager;
	Camera* camera;
	Light* light;
};

