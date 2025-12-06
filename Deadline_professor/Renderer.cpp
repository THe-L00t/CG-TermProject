#include "Renderer.h"
#include "Camera.h"
#include "Light.h"

Renderer* Renderer::activeInstance = nullptr;

Renderer::Renderer(ResourceManager* resMgr) : resourceManager(resMgr), camera(nullptr), light(nullptr)
{

}

Renderer::~Renderer()
{
	Deactive();
}

void Renderer::Init()
{
	std::cout << "Renderer: Initializing..." << std::endl;

	Active();

	// OpenGL 버전 확인
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	std::cout << "OpenGL Version: " << (version ? (const char*)version : "Unknown") << std::endl;
	std::cout << "GLSL Version: " << (glslVersion ? (const char*)glslVersion : "Unknown") << std::endl;

	// 배경색 설정 (어두운 회색 - 렌더링 확인용)
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	std::cout << "Renderer: Clear color set to (0.2, 0.2, 0.2)" << std::endl;

	// Viewport 초기화 (기본 1920x1080)
	glViewport(0, 0, 1920, 1080);
	std::cout << "Viewport initialized: 1920x1080" << std::endl;

	// 셰이더 로드
	if (!LoadShader("basic", ".\\Shaders\\basic.vert", ".\\Shaders\\basic.frag")) {
		std::cerr << "ERROR: Failed to load shader 'basic'" << std::endl;
	} else {
		std::cout << "Renderer: Shader 'basic' loaded successfully" << std::endl;
	}

	if (!LoadShader("professor", ".\\Shaders\\professor.vert", ".\\Shaders\\professor.frag")) {
		std::cerr << "ERROR: Failed to load shader 'professor'" << std::endl;
	} else {
		std::cout << "Renderer: Shader 'professor' loaded successfully" << std::endl;
	}

	std::cout << "Renderer: Initialization completed" << std::endl;
}

void Renderer::Active()
{
	activeInstance = this;
}

void Renderer::Deactive()
{
	if (activeInstance == this) {
		activeInstance = nullptr;
	}
}

void Renderer::SetCamera(Camera* cam)
{
	camera = cam;
}

void Renderer::SetLight(Light* l)
{
	light = l;
}

void Renderer::OnWindowResize(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Renderer::DrawScene(GLvoid)
{
	// 화면 클리어
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (activeInstance && activeInstance->onDrawScene) {
		activeInstance->onDrawScene();
	} else {
		std::cerr << "DrawScene: No active instance or callback" << std::endl;
	}

	glutSwapBuffers();
}

bool Renderer::LoadShader(const std::string& name, const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
{
	Shader shader;
	if (shader.CompileShader(vsPath, fsPath)) {
		shaders[name] = std::move(shader);
		std::cout << "Shader '" << name << "' loaded successfully" << std::endl;
		return true;
	}
	std::cerr << "Failed to load shader '" << name << "'" << std::endl;
	return false;
}

Shader* Renderer::GetShader(const std::string& name)
{
	auto it = shaders.find(name);
	if (it != shaders.end()) {
		return &(it->second);
	}
	return nullptr;
}

// ============================================
// OBJ 렌더링 함수
// ============================================

void Renderer::RenderObj(const std::string_view& objName, const glm::mat4& modelMatrix)
{
	RenderObj(objName, modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
}

void Renderer::RenderObj(const std::string_view& objName, const glm::mat4& modelMatrix, const glm::vec3& color)
{
	const ObjData* objData = resourceManager->GetObjData(objName);
	if (!objData) {
		std::cerr << "RenderObj: OBJ '" << objName << "' not found!" << std::endl;
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	// 다중 조명 적용
	ApplyLightsToShader(shader);

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 레거시 조명 (하위 호환성 - 다중 조명이 없을 때 사용)
	glm::vec3 lightPos = light ? light->GetPosition() : glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor = light ? light->GetDiffuse() : glm::vec3(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);
	shader->setUniform("uUseTexture", false);
	shader->setUniform("uTextureTiling", glm::vec2(1.0f, 1.0f));
	shader->setUniform("uUseSkinning", false);

	// 개별 VAO 사용
	glBindVertexArray(objData->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(objData->indexCount), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	shader->Unuse();
}

void Renderer::RenderObjWithTexture(const std::string_view& objName, const std::string_view& textureName, const glm::mat4& modelMatrix)
{
	const ObjData* objData = resourceManager->GetObjData(objName);
	if (!objData) {
		std::cerr << "RenderObjWithTexture: OBJ '" << objName << "' not found!" << std::endl;
		return;
	}

	GLuint textureID = resourceManager->GetTexture(textureName);
	if (textureID == 0) {
		std::cerr << "RenderObjWithTexture: Texture '" << textureName << "' not found, falling back to color" << std::endl;
		RenderObj(objName, modelMatrix);
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	// 다중 조명 적용
	ApplyLightsToShader(shader);

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 레거시 조명 (하위 호환성)
	glm::vec3 lightPos = light ? light->GetPosition() : glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor = light ? light->GetDiffuse() : glm::vec3(1.0f, 1.0f, 1.0f);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader->setUniform("uTexture", 0);
	shader->setUniform("uUseTexture", true);
	shader->setUniform("uTextureTiling", glm::vec2(1.0f, 1.0f));
	shader->setUniform("uUseSkinning", false);

	// 개별 VAO 사용
	glBindVertexArray(objData->VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(objData->indexCount), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	shader->Unuse();
}

void Renderer::RenderObjWithTextureTiled(const std::string_view& objName, const std::string_view& textureName, const glm::mat4& modelMatrix, const glm::vec2& tiling)
{
	static bool debugPrinted = false;
	const ObjData* objData = resourceManager->GetObjData(objName);
	if (!objData) {
		std::cerr << "RenderObjWithTextureTiled: OBJ '" << objName << "' not found!" << std::endl;
		return;
	}

	if (!debugPrinted) {
		std::cout << "RenderObjWithTextureTiled: OBJ '" << objName << "' found!" << std::endl;
		std::cout << "  VAO: " << objData->VAO << ", indexCount: " << objData->indexCount << std::endl;
		debugPrinted = true;
	}

	GLuint textureID = resourceManager->GetTexture(textureName);
	if (textureID == 0) {
		std::cerr << "RenderObjWithTextureTiled: Texture '" << textureName << "' not found, using white color" << std::endl;
	} else {
		static bool texDebugPrinted = false;
		if (!texDebugPrinted) {
			std::cout << "RenderObjWithTextureTiled: Texture '" << textureName << "' found! ID: " << textureID << std::endl;
			texDebugPrinted = true;
		}
	}

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	// 다중 조명 적용
	ApplyLightsToShader(shader);

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 레거시 조명 (하위 호환성)
	glm::vec3 lightPos = light ? light->GetPosition() : glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor = light ? light->GetDiffuse() : glm::vec3(1.0f, 1.0f, 1.0f);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	shader->setUniform("uTextureTiling", tiling);
	shader->setUniform("uUseSkinning", false);

	if (textureID != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		shader->setUniform("uTexture", 0);
		shader->setUniform("uUseTexture", true);
	} else {
		shader->setUniform("uUseTexture", false);
		shader->setUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));
	}

	// 개별 VAO 사용
	static bool drawDebugPrinted = false;
	if (!drawDebugPrinted) {
		std::cout << "RenderObjWithTextureTiled: About to draw..." << std::endl;
		std::cout << "  Binding VAO: " << objData->VAO << std::endl;
		std::cout << "  Drawing " << objData->indexCount << " indices" << std::endl;
		drawDebugPrinted = true;
	}

	glBindVertexArray(objData->VAO);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error after glBindVertexArray: " << err << std::endl;
	}

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(objData->indexCount), GL_UNSIGNED_INT, 0);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL Error after glDrawElements: " << err << std::endl;
	}

	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	shader->Unuse();
}

// ============================================
// FBX 렌더링 함수 (텍스처 포함)
// ============================================

// FBX 정적 메시 렌더링 (텍스처 포함)
// FBX 정적 메시 렌더링 (텍스처 포함)
void Renderer::RenderFBX(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix)
{
	const FBXModel* model = resourceManager->GetFBXModel(modelName);
	if (!model || model->meshes.empty()) {
		std::cerr << "RenderFBX: Model '" << modelName << "' not found or empty!" << std::endl;
		return;
	}

	GLuint textureID = resourceManager->GetTexture(textureName);
	if (textureID == 0) {
		std::cerr << "RenderFBX: Texture '" << textureName << "' not found (ID=0)" << std::endl;
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	// 다중 조명 적용
	ApplyLightsToShader(shader);

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 레거시 조명 (하위 호환성)
	glm::vec3 lightPos = light ? light->GetPosition() : glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor = light ? light->GetDiffuse() : glm::vec3(1.0f, 1.0f, 1.0f);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);
	shader->setUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader->setUniform("uTexture", 0);
	shader->setUniform("uUseTexture", true);
	shader->setUniform("uTextureTiling", glm::vec2(1.0f, 1.0f));
	shader->setUniform("uUseSkinning", false);

	for (const auto& mesh : model->meshes) {
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	shader->Unuse();
}

// FBX 애니메이션 메시 렌더링 (텍스처 포함)
void Renderer::RenderFBXAnimated(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix, const std::vector<glm::mat4>& boneTransforms)
{
	const FBXModel* model = resourceManager->GetFBXModel(modelName);
	if (!model || model->meshes.empty()) {
		std::cerr << "RenderFBXAnimated: Model '" << modelName << "' not found or empty" << std::endl;
		return;
	}

	GLuint textureID = resourceManager->GetTexture(textureName);
	if (textureID == 0) {
		std::cerr << "RenderFBXAnimated: Texture '" << textureName << "' not found (ID=0)" << std::endl;
		return;
	}

	Shader* shader = GetShader("professor");
	if (!shader) {
		std::cerr << "Professor shader not found, falling back to basic" << std::endl;
		shader = GetShader("basic");
		if (!shader) return;
	}

	shader->Use();

	// 다중 조명 적용
	ApplyLightsToShader(shader);

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 레거시 조명 (하위 호환성)
	glm::vec3 lightPos = light ? light->GetPosition() : glm::vec3(0.0f, 10.0f, 0.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor = light ? light->GetDiffuse() : glm::vec3(1.0f, 1.0f, 1.0f);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);
	shader->setUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader->setUniform("uTexture", 0);
	shader->setUniform("uUseTexture", true);

	bool useSkinning = !boneTransforms.empty();
	shader->setUniform("uUseSkinning", useSkinning);

	if (useSkinning) {
		int boneCount = std::min(static_cast<int>(boneTransforms.size()), 100);
		for (int i = 0; i < boneCount; ++i) {
			shader->setUniform("uBoneTransforms[" + std::to_string(i) + "]", boneTransforms[i]);
		}
	}

	for (const auto& mesh : model->meshes) {
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	shader->Unuse();
}

// ============================================
// 다중 조명 시스템
// ============================================

void Renderer::AddLight(Light* light)
{
	if (!light) return;

	// 중복 추가 방지
	auto it = std::find(lights.begin(), lights.end(), light);
	if (it == lights.end()) {
		lights.push_back(light);
		std::cout << "Renderer: Light added. Total lights: " << lights.size() << std::endl;
	}
}

void Renderer::RemoveLight(Light* light)
{
	if (!light) return;

	auto it = std::find(lights.begin(), lights.end(), light);
	if (it != lights.end()) {
		lights.erase(it);
		std::cout << "Renderer: Light removed. Total lights: " << lights.size() << std::endl;
	}
}

void Renderer::ClearLights()
{
	lights.clear();
	std::cout << "Renderer: All lights cleared" << std::endl;
}

void Renderer::ApplyLightsToShader(Shader* shader) const
{
	if (!shader) return;

	// 활성 광원 개수 설정
	int activeLightCount = 0;
	for (const auto lightPtr : lights) {
		if (lightPtr && lightPtr->IsEnabled()) {
			activeLightCount++;
		}
	}

	// 최대 광원 개수 제한
	const int MAX_LIGHTS = 8;
	activeLightCount = std::min(activeLightCount, MAX_LIGHTS);

	shader->setUniform("uLightCount", activeLightCount);

	// 활성 광원만 순서대로 셰이더에 전달
	int lightIndex = 0;
	for (const auto lightPtr : lights) {
		if (!lightPtr || !lightPtr->IsEnabled()) continue;
		if (lightIndex >= MAX_LIGHTS) break;

		lightPtr->ApplyToShader(shader->GetProgram(), lightIndex);
		lightIndex++;
	}
}

void Renderer::RenderLightDebugPoints()
{
	if (lights.empty()) return;

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);
	shader->setUniform("uModel", glm::mat4(1.0f));
	shader->setUniform("uUseTexture", false);
	shader->setUniform("uUseSkinning", false);

	shader->setUniform("uLightCount", 0);
	shader->setUniform("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setUniform("uLightPos", glm::vec3(0.0f, 0.0f, 0.0f));

	glPointSize(15.0f);
	glDisable(GL_DEPTH_TEST);

	// 모든 활성 광원의 위치 수집
	std::vector<glm::vec3> positions;
	for (const auto& lightPtr : lights) {
		if (lightPtr && lightPtr->IsEnabled()) {
			positions.push_back(lightPtr->GetPosition());
		}
	}

	if (positions.empty()) {
		glEnable(GL_DEPTH_TEST);
		glPointSize(1.0f);
		shader->Unuse();
		return;
	}

	// VAO, VBO 생성
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	shader->setUniform("uColor", glm::vec3(1.0f, 1.0f, 1.0f));

	glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(positions.size()));

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glEnable(GL_DEPTH_TEST);
	glPointSize(1.0f);

	shader->Unuse();
}
