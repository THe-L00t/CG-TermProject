#include "Renderer.h"
#include "Camera.h"

Renderer* Renderer::activeInstance = nullptr;

Renderer::Renderer(ResourceManager* resMgr) : resourceManager(resMgr), camera(nullptr)
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

	// 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// 셰이더 로드
	std::filesystem::path currentPath = std::filesystem::current_path();
	std::filesystem::path vertPath = currentPath / "Shaders" / "basic.vert";
	std::filesystem::path fragPath = currentPath / "Shaders" / "basic.frag";

	if (!LoadShader("basic", vertPath, fragPath)) {
		std::cerr << "ERROR: Failed to load shader 'basic'" << std::endl;
		std::cerr << "Check if basic.vert and basic.frag exist in: " << currentPath / "Shaders" << std::endl;
	} else {
		std::cout << "Renderer: Shader 'basic' loaded successfully" << std::endl;
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

void Renderer::RenderFBXModel(const std::string_view& modelName, const glm::mat4& modelMatrix)
{
	const FBXModel* model = resourceManager->GetFBXModel(modelName);
	if (!model || model->meshes.empty()) {
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) {
		return;
	}

	shader->Use();

	// 변환 행렬 설정
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	if (camera) {
		view = camera->GetViewMat();
		projection = camera->GetProjMat();
	}

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);
	shader->setUniform("uUseTexture", false); // 텍스처 미사용

	// 모든 메시 렌더링
	for (const auto& mesh : model->meshes) {
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	shader->Unuse();
}

void Renderer::RenderFBXModelWithAnimation(const std::string_view& modelName, const glm::mat4& modelMatrix, const std::vector<glm::mat4>& boneTransforms)
{
	const FBXModel* model = resourceManager->GetFBXModel(modelName);
	if (!model || model->meshes.empty()) {
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) {
		return;
	}

	shader->Use();

	// 변환 행렬 설정
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	if (camera) {
		view = camera->GetViewMat();
		projection = camera->GetProjMat();
	}

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);
	shader->setUniform("uUseTexture", false); // 텍스처 미사용

	// 스켈레탈 애니메이션 설정
	bool useSkinning = !boneTransforms.empty();
	shader->setUniform("uUseSkinning", useSkinning);

	if (useSkinning) {
		// 본 변환 행렬 전달 (최대 100개)
		int boneCount = std::min(static_cast<int>(boneTransforms.size()), 100);
		for (int i = 0; i < boneCount; ++i) {
			std::string uniformName = "uBoneTransforms[" + std::to_string(i) + "]";
			shader->setUniform(uniformName, boneTransforms[i]);
		}
	}

	// 모든 메시 렌더링
	for (const auto& mesh : model->meshes) {
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	shader->Unuse();
}

void Renderer::RenderFBXModelWithTexture(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix)
{
	const FBXModel* model = resourceManager->GetFBXModel(modelName);
	if (!model || model->meshes.empty()) {
		std::cerr << "RenderFBXModelWithTexture: Model '" << modelName << "' not found or empty" << std::endl;
		return;
	}

	GLuint textureID = resourceManager->GetTexture(textureName);
	if (textureID == 0) {
		std::cerr << "RenderFBXModelWithTexture: Texture '" << textureName << "' not found (ID=0), falling back to no texture" << std::endl;
		RenderFBXModel(modelName, modelMatrix);
		return;
	}

	std::cout << "RenderFBXModelWithTexture: Using texture ID " << textureID << " for model '" << modelName << "'" << std::endl;

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", glm::vec3(1.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader->setUniform("uTexture", 0);
	shader->setUniform("uUseTexture", true);

	for (const auto& mesh : model->meshes) {
		glBindVertexArray(mesh.VAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	shader->Unuse();
}

void Renderer::RenderFBXModelWithAnimationAndTexture(const std::string_view& modelName, const std::string_view& textureName, const glm::mat4& modelMatrix, const std::vector<glm::mat4>& boneTransforms)
{
	const FBXModel* model = resourceManager->GetFBXModel(modelName);
	if (!model || model->meshes.empty()) {
		std::cerr << "RenderFBXModelWithAnimationAndTexture: Model '" << modelName << "' not found or empty" << std::endl;
		return;
	}

	GLuint textureID = resourceManager->GetTexture(textureName);
	if (textureID == 0) {
		std::cerr << "RenderFBXModelWithAnimationAndTexture: Texture '" << textureName << "' not found (ID=0), falling back to no texture" << std::endl;
		RenderFBXModelWithAnimation(modelName, modelMatrix, boneTransforms);
		return;
	}

	std::cout << "RenderFBXModelWithAnimationAndTexture: Using texture ID " << textureID << " for model '" << modelName << "'" << std::endl;

	Shader* shader = GetShader("basic");
	if (!shader) return;

	shader->Use();

	glm::mat4 view = camera ? camera->GetViewMat() : glm::mat4(1.0f);
	glm::mat4 projection = camera ? camera->GetProjMat() : glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", modelMatrix);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", glm::vec3(1.0f));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	shader->setUniform("uTexture", 0);
	shader->setUniform("uUseTexture", true);
	std::cout << "DEBUG: Texture bound to GL_TEXTURE0, uUseTexture set to TRUE" << std::endl;

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
