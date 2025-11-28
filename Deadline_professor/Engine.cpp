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
	glEnable(GL_CULL_FACE);

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
		if (frameCount % 60 == 0) {
			std::cout << "Frame " << frameCount << ": Rendering..." << std::endl;
		}
		frameCount++;

		// RunLee 애니메이션 렌더링
		const XMeshData* meshData = resourceManager->GetXMeshData("RunLee");
		if (meshData && meshData->has_skeleton && animPlayer->IsPlaying()) {
			if (frameCount % 60 == 0) {
				std::cout << "Rendering animated RunLee" << std::endl;
			}
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f)); // 스케일 조정
			r->RenderAnimatedMesh("RunLee", animPlayer->GetFinalTransforms(), model);
		}
		else if (meshData) {
			// 애니메이션이 없으면 정적 메시로 렌더링
			if (frameCount % 60 == 0) {
				std::cout << "Rendering static RunLee" << std::endl;
			}
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
			r->RenderXMesh("RunLee", model);
		}
		else {
			// RunLee 로드 실패 시 기본 큐브 렌더링
			if (frameCount % 60 == 0) {
				std::cout << "Rendering fallback cube" << std::endl;
			}
			r->RenderTestCube();
		}
		};

	// GLUT 콜백 등록
	glutDisplayFunc(Renderer::DrawScene);
	glutReshapeFunc(Window::Resize);
	glutKeyboardFunc(InputManager::Keyboard);
	glutSpecialFunc(InputManager::SKeyboard);
	glutMouseFunc(InputManager::Mouse);
	glutTimerFunc(1, TimerCallback, 0);

	std::cout << "=== Engine Initialization Complete ===" << std::endl;
}

void Engine::LoadAssets()
{
	std::cout << "=== Loading Assets ===" << std::endl;

	// OBJ 파일 로드
	if (!resourceManager->LoadObj("cube", "cube.obj")) {
		std::cerr << "Warning: Failed to load cube.obj" << std::endl;
	}

	// XMesh 파일 로드
	if (!resourceManager->LoadXMesh("RunLee", "RunLee.xmesh")) {
		std::cerr << "Warning: Failed to load RunLee.xmesh" << std::endl;
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
