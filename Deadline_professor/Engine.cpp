#include "Engine.h"
#include "Window.h"
#include "Renderer.h"
#include "GameTimer.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "AnimationPlayer.h"

Engine* Engine::instance = nullptr;
Engine* g_engine = nullptr;

Engine::Engine()
{
	instance = this;
	g_engine = this;
}

Engine::~Engine()
{
	if (instance == this) {
		instance = nullptr;
	}
}

void Engine::Initialize(int argc, char** argv)
{
	glutInit(&argc, argv);
	w = std::make_unique<Window>();

	w->Create();

	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}

	// 리소스 매니저 초기화 및 에셋 미리 로드
	resourceManager = std::make_unique<ResourceManager>();
	resourceManager->Active();
	LoadAssets();

	// 렌더러 초기화
	r = std::make_unique<Renderer>(resourceManager.get());
	r->Init();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);  // Face culling 일시적으로 비활성화 (winding order 확인용)

	std::cout << "OpenGL settings:" << std::endl;
	std::cout << "  DEPTH_TEST: enabled" << std::endl;
	std::cout << "  CULL_FACE: disabled (for debugging)" << std::endl;

	gameTimer = std::make_unique<GameTimer>();

	// Camera 초기화
	camera = std::make_unique<Camera>(
		glm::vec3(0.0f, 2.0f, 5.0f),  // position
		glm::vec3(0.0f, 0.0f, 0.0f),  // target
		glm::vec3(0.0f, 1.0f, 0.0f),  // up
		45.0f,                         // fov
		(float)w->GetWidth() / (float)w->GetHeight()  // aspect ratio
	);
	std::cout << "Camera Initialized" << std::endl;

	// Renderer에 Camera 연결
	r->SetCamera(camera.get());
	std::cout << "Renderer Connected to Camera" << std::endl;

	// InputManager 초기화 및 Camera, Window 연결
	inputManager = std::make_unique<InputManager>();
	inputManager->init();
	inputManager->SetCamera(camera.get());
	inputManager->SetWindow(w.get());

	// 키보드 액션 바인딩
	inputManager->ActionW = [this]() {
		camera->MoveForward(gameTimer->elapsedTime);
	};
	inputManager->ActionS = [this]() {
		camera->MoveBackward(gameTimer->elapsedTime);
	};
	inputManager->ActionA = [this]() {
		camera->MoveLeft(gameTimer->elapsedTime);
	};
	inputManager->ActionD = [this]() {
		camera->MoveRight(gameTimer->elapsedTime);
	};
	inputManager->ActionWheelUp = [this]() {
		camera->Zoom(1.0f);
	};
	inputManager->ActionWheelDown = [this]() {
		camera->Zoom(-1.0f);
	};

	std::cout << "InputManager Initialized and Connected to Camera & Window" << std::endl;

	// AnimationPlayer 초기화
	animPlayer = std::make_unique<AnimationPlayer>();
	std::cout << "AnimationPlayer Initialized" << std::endl;

	// SceneManager 초기화
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->ChangeScene("Test");
	std::cout << "SceneManager Initialized with TestScene" << std::endl;

	w->onResize = [this](int w, int h) {
		r->OnWindowResize(w, h);
		};

	r->onDrawScene = [this]() {
		static int frameCount = 0;
		static bool printedOnce = false;

		if (frameCount % 60 == 0) {
			std::cout << "Frame " << frameCount << ": Rendering..." << std::endl;
		}
		frameCount++;

		// RunLee 애니메이션 렌더링
		const XMeshData* meshData = resourceManager->GetXMeshData("RunLee");

		// 첫 프레임에만 상세 정보 출력
		if (!printedOnce) {
			std::cout << "\n=== XMesh Rendering Debug Info ===" << std::endl;

			// 카메라 정보 출력
			glm::vec3 camPos = camera->GetPosition();
			glm::vec3 camDir = camera->GetDirection();
			std::cout << "Camera Position: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")" << std::endl;
			std::cout << "Camera Direction: (" << camDir.x << ", " << camDir.y << ", " << camDir.z << ")" << std::endl;

			if (meshData) {
				std::cout << "RunLee mesh found!" << std::endl;
				std::cout << "  Index count: " << meshData->index_count << std::endl;
				std::cout << "  Vertex streams: " << meshData->streams.size() << std::endl;
				std::cout << "  Has skeleton: " << (meshData->has_skeleton ? "Yes" : "No") << std::endl;
				std::cout << "  Bone count: " << meshData->bones.size() << std::endl;
				std::cout << "  Sections: " << meshData->sections.size() << std::endl;
				if (meshData->has_skeleton) {
					std::cout << "  Animation playing: " << (animPlayer->IsPlaying() ? "Yes" : "No") << std::endl;
				}
			} else {
				std::cout << "RunLee mesh NOT FOUND!" << std::endl;
			}
			std::cout << "==================================\n" << std::endl;
			printedOnce = true;
		}

		// 🔍 임시: 애니메이션 끄고 정적 메시로 테스트
		if (false && meshData && meshData->has_skeleton && animPlayer->IsPlaying()) {
			if (frameCount % 60 == 0) {
				std::cout << "Rendering animated RunLee" << std::endl;
			}
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 원점
			model = glm::scale(model, glm::vec3(2.0f)); // 2m 크기
			r->RenderAnimatedMesh("RunLee", animPlayer->GetFinalTransforms(), model);
		}
		else if (meshData) {
			// 애니메이션이 없으면 정적 메시로 렌더링
			std::cout << "🔍 Frame " << frameCount << ": Calling RenderXMesh..." << std::endl;

			glm::mat4 model = glm::mat4(1.0f);
			// ✅ 모델을 카메라 정면에 배치
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
			model = glm::scale(model, glm::vec3(2.0f));

			std::cout << "  Model matrix: scale=2.0, pos=(0,0,0)" << std::endl;
			r->RenderXMesh("RunLee", model);
			std::cout << "  RenderXMesh returned" << std::endl;
		} else {
			std::cout << "❌ meshData is NULL!" << std::endl;
		}
		};

	// GLUT 콜백 등록
	glutDisplayFunc(Renderer::DrawScene);
	glutReshapeFunc(Window::Resize);
	glutKeyboardFunc(InputManager::Keyboard);
	glutSpecialFunc(InputManager::SKeyboard);
	glutMouseFunc(InputManager::Mouse);
	glutPassiveMotionFunc(InputManager::PassiveMotion);
	glutMotionFunc(InputManager::PassiveMotion); // Also handle when buttons are pressed
	glutTimerFunc(1, TimerCallback, 0);

	std::cout << "=== Engine Initialization Complete ===" << std::endl;
}

void Engine::LoadAssets()
{
	std::cout << "=== Loading Assets ===" << std::endl;

	// OBJ 파일 로드 (fallback용)
	if (!resourceManager->LoadObj("bugatti", "bugatti.obj")) {
		std::cerr << "Warning: Failed to load bugatti.obj" << std::endl;
	}

	// XMesh 파일 로드
	std::cout << "\n--- Loading XMesh files ---" << std::endl;

	if (!resourceManager->LoadXMesh("RunLee", "RunLee.xmesh")) {
		std::cerr << "ERROR: Failed to load RunLee.xmesh" << std::endl;
	} else {
		std::cout << "SUCCESS: RunLee.xmesh loaded" << std::endl;
	}

	if (!resourceManager->LoadXMesh("RunSong", "RunSong.xmesh")) {
		std::cerr << "Warning: Failed to load RunSong.xmesh" << std::endl;
	}

	if (!resourceManager->LoadXMesh("RunDragon", "RunDragon.xmesh")) {
		std::cerr << "Warning: Failed to load RunDragon.xmesh" << std::endl;
	}

	std::cout << "=== Assets Loaded ===" << std::endl;
}

void Engine::Run()
{
	
	glutMainLoop();
}

void Engine::Update()
{
	gameTimer->Update();
	float deltaTime = gameTimer->elapsedTime;

	// SceneManager 업데이트
	if (sceneManager) {
		sceneManager->update(deltaTime);
	}

	glutPostRedisplay();
}

void Engine::TimerCallback(int value)
{
	if (instance) {
		instance->Update();
		glutTimerFunc(1, TimerCallback, 0);
	}
}
