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
	void SetLight(Light* l);  // 레거시 단일 조명 (하위호환성)

	void OnWindowResize(int, int);

	// 다중 조명 관리
	void AddLight(Light* light);
	void RemoveLight(Light* light);
	void ClearLights();
	const std::vector<Light*>& GetLights() const { return lights; }
	void ApplyLightsToShader(Shader* shader) const;
	// DEBUG: 조명 위치 렌더링
	void RenderLightDebugPoints();

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

	//화면에 텍스쳐 띄우기 
	void InitScreenQuad(const glm::vec2& RT, const glm::vec2& LB);
	void RenderTextrue(const std::string_view& textureName);

	void InituiQuad(const glm::vec2& RT, const glm::vec2& LB);
	void Renderui(const std::string_view& textureName, float deltatime);

	void InitPostQuad();
	void RenderFinal();
	void SelectFBO();
	void SelectScreen();
private:
	// OBJ 렌더링 헬퍼 함수
	void ConfigureSharedVAOForOBJ(const ObjData* objData) const;

	static Renderer* activeInstance;

	std::unordered_map<std::string, Shader> shaders;
	ResourceManager* resourceManager;
	Camera* camera;
	Light* light;  // 레거시 단일 조명 (하위호환성)
	std::vector<Light*> lights;  // 다중 조명 시스템

	// 전체 화면 출력 관리용
	GLuint ScreenVAO{};
	GLuint ScreenVBO{};

	GLuint uiVAO{};
	GLuint uiVBO{};

	// 후처리용
	GLuint postVAO{};
	GLuint postVBO{};
	GLuint postFBO{};
	GLuint postTexture{};
};

