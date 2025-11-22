#include "Renderer.h"

Renderer* Renderer::activeInstance = nullptr;

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	Deactive();
}

void Renderer::Init()
{
	Active();
	reManager.Active();

	// 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// 셰이더 로드
	LoadShader("basic", "basic.vert", "basic.frag");

	// 테스트 OBJ 로드
	LoadTestObj("cube", "cube.obj");

	// OBJ 데이터 가져오기
	const ObjData* cubeData = reManager.GetObjData("cube");
	if (cubeData) {
		testVAO = reManager.GetVAO();
		testVBO = cubeData->VBO;
		testEBO = cubeData->EBO;
		testIndexCount = cubeData->indexCount;

		std::cout << "Test cube ready with " << testIndexCount << " indices" << std::endl;
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

void Renderer::OnWindowResize(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Renderer::DrawScene(GLvoid)
{
	if (activeInstance->onDrawScene) {
		activeInstance->onDrawScene();
	}

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

bool Renderer::LoadTestObj(const std::string& name, const std::filesystem::path& path)
{
	return reManager.LoadObj(name, path);
}

void Renderer::RenderTestCube()
{
	Shader* shader = GetShader("basic");
	if (!shader) {
		std::cerr << "Shader 'basic' not found" << std::endl;
		return;
	}

	shader->Use();

	// 변환 행렬 설정
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, (float)glutGet(GLUT_ELAPSED_TIME) * 0.001f, glm::vec3(0.5f, 1.0f, 0.0f));

	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

	shader->setUniform("uModel", model);
	shader->setUniform("uView", view);
	shader->setUniform("uProjection", projection);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// 렌더링
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (testVAO != 0) {
		glBindVertexArray(testVAO);
		glDrawElements(GL_TRIANGLES, testIndexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	shader->Unuse();
	glutSwapBuffers();
}

