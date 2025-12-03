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
	std::cout << "\n========================================" << std::endl;
	std::cout << "RENDERER::INIT() CALLED" << std::endl;
	std::cout << "========================================\n" << std::endl;

	Active();

	// 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// 현재 작업 디렉토리 확인
	std::filesystem::path currentPath = std::filesystem::current_path();
	std::cout << "Current working directory: " << currentPath << std::endl;

	// 셰이더 파일 경로 확인
	std::filesystem::path vertPath = currentPath / "basic.vert";
	std::filesystem::path fragPath = currentPath / "basic.frag";

	std::cout << "Looking for vertex shader: " << vertPath << std::endl;
	std::cout << "  Exists: " << (std::filesystem::exists(vertPath) ? "YES" : "NO") << std::endl;
	std::cout << "Looking for fragment shader: " << fragPath << std::endl;
	std::cout << "  Exists: " << (std::filesystem::exists(fragPath) ? "YES" : "NO") << std::endl;

	// 셰이더 로드
	std::cout << "\n========================================" << std::endl;
	std::cout << "ATTEMPTING TO LOAD SHADER 'basic'" << std::endl;
	std::cout << "========================================" << std::endl;

	bool shaderLoaded = LoadShader("basic", vertPath, fragPath);

	if (shaderLoaded) {
		std::cout << "✅ SUCCESS: Shader 'basic' loaded!" << std::endl;
		std::cout << "   Total shaders in map: " << shaders.size() << std::endl;
	} else {
		std::cerr << "❌ CRITICAL ERROR: Failed to load shader 'basic'!" << std::endl;
		std::cerr << "   The program will not render correctly." << std::endl;
		std::cerr << "   Check if basic.vert and basic.frag exist in the working directory." << std::endl;
	}
	std::cout << "========================================\n" << std::endl;

	std::cout << "Renderer::Init() completed. Shaders loaded: " << shaders.size() << std::endl;

	// OBJ 데이터 가져오기 (이미 Engine에서 로드됨)
	const ObjData* cubeData = resourceManager->GetObjData("bugatti");
	if (cubeData) {
		testVAO = resourceManager->GetVAO();
		testVBO = cubeData->VBO;
		testEBO = cubeData->EBO;
		testIndexCount = cubeData->indexCount;

		std::cout << "Test object ready with " << testIndexCount << " indices" << std::endl;
	}
	else {
		std::cerr << "Failed to get bugatti data from ResourceManager" << std::endl;
	}
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

void Renderer::RenderTestCube()
{
	std::cout << "RenderTestCube called" << std::endl;

	Shader* shader = GetShader("basic");
	if (!shader) {
		std::cerr << "Shader 'basic' not found" << std::endl;
		return;
	}

	shader->Use();

	// 변환 행렬 설정
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
	model = glm::rotate(model, (float)glutGet(GLUT_ELAPSED_TIME) * 0.001f, glm::vec3(0.5f, 1.0f, 0.0f));

	// Camera에서 view, projection 행렬 가져오기
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	if (camera) {
		view = camera->GetViewMat();
		projection = camera->GetProjMat();
	}

	shader->setUniform("uModel", model);
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

	// 렌더링
	if (testVAO != 0) {
		std::cout << "Drawing cube with " << testIndexCount << " indices" << std::endl;
		glBindVertexArray(testVAO);
		glDrawElements(GL_TRIANGLES, testIndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	} else {
		std::cerr << "testVAO is 0" << std::endl;
	}

	shader->Unuse();
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
