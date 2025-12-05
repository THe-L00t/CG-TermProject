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

	void OnWindowResize(int, int);

	//콜백 함수
	std::function<void(GLvoid)> onDrawScene;
	static void DrawScene(GLvoid);

	// 셰이더 관리
	bool LoadShader(const std::string&, const std::filesystem::path&, const std::filesystem::path&);
	Shader* GetShader(const std::string&);

	// FBX 모델 렌더링
	void RenderFBXModel(const std::string_view& modelName, const glm::mat4& modelMatrix = glm::mat4(1.0f));
	void RenderFBXModelWithAnimation(const std::string_view& modelName, const glm::mat4& modelMatrix, const std::vector<glm::mat4>& boneTransforms);

	// 텍스처 적용 렌더링
	void RenderFBXModelWithTexture(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix = glm::mat4(1.0f));
	void RenderFBXModelWithAnimationAndTexture(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix, const std::vector<glm::mat4>& boneTransforms);

	// OBJ 모델 렌더링
	void RenderObjModel(const std::string_view& modelName, const glm::mat4& modelMatrix = glm::mat4(1.0f));
	void RenderObjModelWithTexture(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix = glm::mat4(1.0f));

	// 조명 관리
	void AddLight(Light* light);
	void RemoveLight(Light* light);
	void ClearLights();
	const std::vector<Light*>& GetLights() const { return lights; }
	void ApplyLightsToShader(Shader* shader) const;
	// DEBUG: 조명 위치 렌더링
	void RenderLightDebugPoints();

private:
	// OBJ 렌더링 헬퍼 함수
	void ConfigureSharedVAOForOBJ(const ObjData* objData) const;

	static Renderer* activeInstance;

	std::unordered_map<std::string, Shader> shaders;
	ResourceManager* resourceManager;
	Camera* camera;
	std::vector<Light*> lights;
};

