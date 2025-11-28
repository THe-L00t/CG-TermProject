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
	Active();

	// 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// 셰이더 로드
	LoadShader("basic", "basic.vert", "basic.frag");

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

void Renderer::RenderXMesh(const std::string_view& meshName, const glm::mat4& modelMatrix)
{
	const XMeshData* meshData = resourceManager->GetXMeshData(meshName);
	if (!meshData) {
		std::cerr << "XMesh '" << meshName << "' not found" << std::endl;
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) {
		std::cerr << "Shader 'basic' not found" << std::endl;
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
	glm::vec3 viewPos(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// VAO 바인드 및 렌더링
	glBindVertexArray(resourceManager->GetVAO());

	// 섹션별로 렌더링 (섹션이 있는 경우)
	if (!meshData->sections.empty()) {
		for (const auto& section : meshData->sections) {
			glDrawElementsBaseVertex(
				GL_TRIANGLES,
				section.index_count,
				meshData->index_type,
				(void*)(section.index_start * (meshData->index_type == GL_UNSIGNED_SHORT ? 2 : 4)),
				section.vertex_start
			);
		}
	}
	else {
		// 섹션 정보가 없으면 전체 인덱스 렌더링
		glDrawElements(GL_TRIANGLES, meshData->index_count, meshData->index_type, 0);
	}

	glBindVertexArray(0);
	shader->Unuse();
}

void Renderer::RenderXMeshSection(const std::string_view& meshName, size_t sectionIndex, const glm::mat4& modelMatrix)
{
	const XMeshData* meshData = resourceManager->GetXMeshData(meshName);
	if (!meshData) {
		std::cerr << "XMesh '" << meshName << "' not found" << std::endl;
		return;
	}

	if (sectionIndex >= meshData->sections.size()) {
		std::cerr << "Section index " << sectionIndex << " out of range for XMesh '" << meshName << "'" << std::endl;
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) {
		std::cerr << "Shader 'basic' not found" << std::endl;
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
	glm::vec3 viewPos(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// VAO 바인드 및 특정 섹션만 렌더링
	glBindVertexArray(resourceManager->GetVAO());

	const MeshSection& section = meshData->sections[sectionIndex];
	glDrawElementsBaseVertex(
		GL_TRIANGLES,
		section.index_count,
		meshData->index_type,
		(void*)(section.index_start * (meshData->index_type == GL_UNSIGNED_SHORT ? 2 : 4)),
		section.vertex_start
	);

	glBindVertexArray(0);
	shader->Unuse();
}

void Renderer::RenderAnimatedMesh(const std::string_view& meshName, const std::vector<glm::mat4>& boneTransforms, const glm::mat4& modelMatrix)
{
	const XMeshData* meshData = resourceManager->GetXMeshData(meshName);
	if (!meshData) {
		std::cerr << "XMesh '" << meshName << "' not found" << std::endl;
		return;
	}

	if (!meshData->has_skeleton) {
		std::cerr << "Mesh '" << meshName << "' has no skeleton" << std::endl;
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) {
		std::cerr << "Shader 'basic' not found" << std::endl;
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

	// 본 변환 행렬 전달 (최대 100개 본 지원)
	const int MAX_BONES = 100;
	int boneCount = std::min(static_cast<int>(boneTransforms.size()), MAX_BONES);

	for (int i = 0; i < boneCount; ++i) {
		std::string uniformName = "uBones[" + std::to_string(i) + "]";
		shader->setUniform(uniformName.c_str(), boneTransforms[i]);
	}

	// 본 개수 전달
	shader->setUniform("uBoneCount", boneCount);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// VAO 바인드 및 렌더링
	glBindVertexArray(resourceManager->GetVAO());

	// 섹션별로 렌더링
	if (!meshData->sections.empty()) {
		for (const auto& section : meshData->sections) {
			glDrawElementsBaseVertex(
				GL_TRIANGLES,
				section.index_count,
				meshData->index_type,
				(void*)(section.index_start * (meshData->index_type == GL_UNSIGNED_SHORT ? 2 : 4)),
				section.vertex_start
			);
		}
	}
	else {
		// 섹션 정보가 없으면 전체 인덱스 렌더링
		glDrawElements(GL_TRIANGLES, meshData->index_count, meshData->index_type, 0);
	}

	glBindVertexArray(0);
	shader->Unuse();
}

