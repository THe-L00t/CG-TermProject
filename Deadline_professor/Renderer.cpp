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

void Renderer::RenderXMesh(const std::string_view& meshName, const glm::mat4& modelMatrix)
{
	static bool printedDebug = false;

	const XMeshData* meshData = resourceManager->GetXMeshData(meshName);
	if (!meshData) {
		std::cerr << "XMesh '" << meshName << "' not found" << std::endl;
		return;
	}

	if (meshData->index_count == 0) {
		std::cerr << "XMesh '" << meshName << "' has no indices" << std::endl;
		return;
	}

	if (meshData->streams.empty()) {
		std::cerr << "XMesh '" << meshName << "' has no vertex streams" << std::endl;
		return;
	}

	if (!printedDebug) {
		std::cout << "\n=== RenderXMesh Debug ===" << std::endl;
		std::cout << "Mesh: " << meshName << std::endl;
		std::cout << "Index count: " << meshData->index_count << std::endl;
		std::cout << "Index type: " << (meshData->index_type == GL_UNSIGNED_SHORT ? "UNSIGNED_SHORT" : "UNSIGNED_INT") << std::endl;
		std::cout << "VBOs: " << meshData->vbos.size() << std::endl;
		std::cout << "EBO: " << meshData->ebo << std::endl;
		std::cout << "VAO: " << meshData->vao << std::endl;
		std::cout << "Sections: " << meshData->sections.size() << std::endl;

		// VBO/EBO가 실제로 생성되었는지 확인
		for (size_t i = 0; i < meshData->vbos.size(); ++i) {
			std::cout << "  VBO[" << i << "]: " << meshData->vbos[i] << std::endl;
		}

		// 스트림 정보 상세 출력
		std::cout << "Streams detail:" << std::endl;
		for (size_t i = 0; i < meshData->streams.size(); ++i) {
			const auto& s = meshData->streams[i];
			std::cout << "  Stream " << i << " (id=" << s.stream_id << "): "
			          << "count=" << s.count << ", element_size=" << s.element_size
			          << ", total_size=" << s.size << std::endl;
		}

		// Quantization 메타데이터 출력
		std::cout << "Quantization metadata:" << std::endl;
		std::cout << "  pos_offset: (" << meshData->pos_offset.x << ", " << meshData->pos_offset.y << ", " << meshData->pos_offset.z << ")" << std::endl;
		std::cout << "  pos_scale: " << meshData->pos_scale << std::endl;

		// 🚨 렌더링 시점에는 stream.data가 무효화됨 (file_buffer 해제됨)
		// GPU VBO에 이미 업로드되어 있으므로 CPU 메모리는 불필요

		// OpenGL 상태 확인
		GLboolean depthTest, cullFace;
		glGetBooleanv(GL_DEPTH_TEST, &depthTest);
		glGetBooleanv(GL_CULL_FACE, &cullFace);
		std::cout << "GL_DEPTH_TEST: " << (depthTest ? "enabled" : "disabled") << std::endl;
		std::cout << "GL_CULL_FACE: " << (cullFace ? "enabled" : "disabled") << std::endl;

		// 카메라 정보
		if (camera) {
			glm::vec3 pos = camera->GetPosition();
			glm::vec3 dir = camera->GetDirection();
			std::cout << "Camera pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
			std::cout << "Camera dir: (" << dir.x << ", " << dir.y << ", " << dir.z << ")" << std::endl;
		}

		std::cout << "========================\n" << std::endl;
		printedDebug = true;
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

	// 애니메이션 없음 (스태틱 메시)
	shader->setUniform("uBoneCount", 0);

	// Position quantization 메타데이터 전달
	shader->setUniform("uPosOffset", meshData->pos_offset);
	shader->setUniform("uPosScale", meshData->pos_scale);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// XMesh 전용 VAO 바인드 (로드 시 이미 설정됨)
	glBindVertexArray(meshData->vao);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "OpenGL error after VAO bind: " << err << std::endl;
	}

	// 섹션별로 렌더링 (섹션이 있는 경우)
	if (!meshData->sections.empty()) {
		if (!printedDebug) {
			std::cout << "Drawing " << meshData->sections.size() << " sections" << std::endl;
		}
		for (size_t i = 0; i < meshData->sections.size(); ++i) {
			const auto& section = meshData->sections[i];
			if (!printedDebug) {
				std::cout << "  Section " << i << ": start=" << section.index_start
				          << ", count=" << section.index_count
				          << ", vertex_start=" << section.vertex_start << std::endl;
			}
			glDrawElementsBaseVertex(
				GL_TRIANGLES,
				section.index_count,
				meshData->index_type,
				(void*)(section.index_start * (meshData->index_type == GL_UNSIGNED_SHORT ? 2 : 4)),
				section.vertex_start
			);

			err = glGetError();
			if (err != GL_NO_ERROR) {
				std::cerr << "OpenGL error after drawing section " << i << ": " << err << std::endl;
			}
		}
	}
	else {
		// 섹션 정보가 없으면 전체 인덱스 렌더링
		if (!printedDebug) {
			std::cout << "Drawing entire mesh: " << meshData->index_count << " indices" << std::endl;
		}
		glDrawElements(GL_TRIANGLES, meshData->index_count, meshData->index_type, 0);

		err = glGetError();
		if (err != GL_NO_ERROR) {
			std::cerr << "OpenGL error after draw: " << err << std::endl;
		}
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

	// 애니메이션 없음 (스태틱 메시)
	shader->setUniform("uBoneCount", 0);

	// Position quantization 메타데이터 전달
	shader->setUniform("uPosOffset", meshData->pos_offset);
	shader->setUniform("uPosScale", meshData->pos_scale);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// XMesh 전용 VAO 바인드 및 특정 섹션만 렌더링
	glBindVertexArray(meshData->vao);

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

	if (boneTransforms.empty()) {
		std::cerr << "❌ Bone transforms are empty" << std::endl;
		return;
	}

	if (meshData->index_count == 0) {
		std::cerr << "❌ Mesh has no indices" << std::endl;
		return;
	}

	Shader* shader = GetShader("basic");
	if (!shader) {
		std::cerr << "❌ Shader 'basic' not found" << std::endl;
		return;
	}

	std::cout << "🔍 Using shader 'basic' for animated mesh" << std::endl;
	shader->Use();

	// Shader 사용 후 에러 체크
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cerr << "❌ OpenGL error after shader use: " << err << std::endl;
	}

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

	// Position quantization 메타데이터 전달
	shader->setUniform("uPosOffset", meshData->pos_offset);
	shader->setUniform("uPosScale", meshData->pos_scale);

	// 라이팅 설정
	glm::vec3 color(0.8f, 0.3f, 0.3f);
	glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
	glm::vec3 viewPos = camera ? camera->GetPosition() : glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

	shader->setUniform("uColor", color);
	shader->setUniform("uLightPos", lightPos);
	shader->setUniform("uViewPos", viewPos);
	shader->setUniform("uLightColor", lightColor);

	// XMesh 전용 VAO 바인드
	glBindVertexArray(meshData->vao);

	// 디버그: 첫 프레임에만 렌더링 정보 출력
	static bool printedRenderInfo = false;
	if (!printedRenderInfo) {
		std::cout << "\n=== RenderAnimatedMesh Debug ===" << std::endl;
		std::cout << "  VAO: " << meshData->vao << std::endl;
		std::cout << "  Index count: " << meshData->index_count << std::endl;
		std::cout << "  Index type: " << (meshData->index_type == GL_UNSIGNED_SHORT ? "UNSIGNED_SHORT" : "UNSIGNED_INT") << std::endl;
		std::cout << "  Bone count: " << boneCount << std::endl;
		std::cout << "  Sections: " << meshData->sections.size() << std::endl;
		if (!meshData->sections.empty()) {
			const auto& sec = meshData->sections[0];
			std::cout << "  Section[0]: start=" << sec.index_start
			          << ", count=" << sec.index_count
			          << ", vstart=" << sec.vertex_start << std::endl;
		}
		std::cout << "  Quantization: offset=(" << meshData->pos_offset.x << ","
		          << meshData->pos_offset.y << "," << meshData->pos_offset.z
		          << "), scale=" << meshData->pos_scale << std::endl;

		// 🔍 Bone 변환 행렬 확인 (첫 3개 본)
		std::cout << "\n  First 3 Bone Transforms:" << std::endl;
		for (int i = 0; i < std::min(3, boneCount); ++i) {
			const glm::mat4& m = boneTransforms[i];
			std::cout << "    Bone[" << i << "]: " << std::endl;
			std::cout << "      [" << m[0][0] << ", " << m[1][0] << ", " << m[2][0] << ", " << m[3][0] << "]" << std::endl;
			std::cout << "      [" << m[0][1] << ", " << m[1][1] << ", " << m[2][1] << ", " << m[3][1] << "]" << std::endl;
			std::cout << "      [" << m[0][2] << ", " << m[1][2] << ", " << m[2][2] << ", " << m[3][2] << "]" << std::endl;
			std::cout << "      [" << m[0][3] << ", " << m[1][3] << ", " << m[2][3] << ", " << m[3][3] << "]" << std::endl;
		}

		std::cout << "================================\n" << std::endl;
		printedRenderInfo = true;
	}

	// 섹션별로 렌더링
	if (!meshData->sections.empty()) {
		std::cout << "🔍 Drawing " << meshData->sections.size() << " sections..." << std::endl;
		for (const auto& section : meshData->sections) {
			std::cout << "  Section: start=" << section.index_start
			          << ", count=" << section.index_count
			          << ", vstart=" << section.vertex_start << std::endl;

			glDrawElementsBaseVertex(
				GL_TRIANGLES,
				section.index_count,
				meshData->index_type,
				(void*)(section.index_start * (meshData->index_type == GL_UNSIGNED_SHORT ? 2 : 4)),
				section.vertex_start
			);

			GLenum err = glGetError();
			if (err != GL_NO_ERROR) {
				std::cerr << "❌ OpenGL error after draw: " << err << std::endl;
			} else {
				std::cout << "  ✅ Draw call succeeded" << std::endl;
			}
		}
	}
	else {
		std::cout << "🔍 Drawing entire mesh: " << meshData->index_count << " indices" << std::endl;
		glDrawElements(GL_TRIANGLES, meshData->index_count, meshData->index_type, 0);

		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			std::cerr << "❌ OpenGL error after draw: " << err << std::endl;
		} else {
			std::cout << "  ✅ Draw call succeeded" << std::endl;
		}
	}

	glBindVertexArray(0);
	shader->Unuse();
}

