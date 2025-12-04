#include "Engine.h"
#include "Window.h"
#include "Renderer.h"
#include "GameTimer.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "FBXAnimationPlayer.h"

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
	// 주의: 실제 게임에서는 Scene이 Player 객체를 소유하므로,
	// Scene에서 InputManager의 액션을 Player에 연결해야 함
	// 여기서는 카메라 직접 제어 (테스트용)
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

	// SceneManager 초기화
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->ChangeScene("Test");
	std::cout << "SceneManager Initialized with TestScene" << std::endl;

	// AnimationPlayer 초기화
	animationPlayer = std::make_unique<FBXAnimationPlayer>();
	const FBXModel* model = resourceManager->GetFBXModel("RunDragon");
	if (model) {
		animationPlayer->Init(model);
		if (!model->animations.empty()) {
			animationPlayer->PlayAnimation(0); // 첫 번째 애니메이션 재생
			std::cout << "AnimationPlayer Initialized with " << model->animations.size() << " animations" << std::endl;
		}
	}

	w->onResize = [this](int w, int h) {
		r->OnWindowResize(w, h);
		};

	// Renderer의 onDrawScene은 SceneManager의 Draw 호출로 변경
	r->onDrawScene = [this]() {
		if (sceneManager) {
			Scene* currentScene = sceneManager->GetCurrentScene();
			if (currentScene) {
				currentScene->Draw();
			}
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

	// FBX 파일 로드
	std::cout << "\n--- Loading FBX files ---" << std::endl;

	if (!resourceManager->LoadFBX("RunLee", "RunLee.fbx")) {
		std::cerr << "ERROR: Failed to load RunLee.fbx" << std::endl;
	} else {
		std::cout << "SUCCESS: RunLee.fbx loaded" << std::endl;
	}

	if (!resourceManager->LoadFBX("RunSong", "RunSong.fbx")) {
		std::cerr << "Warning: Failed to load RunSong.fbx" << std::endl;
	} else {
		std::cout << "SUCCESS: RunSong.fbx loaded" << std::endl;
	}

	if (!resourceManager->LoadFBX("RunDragon", "RunDragon.fbx")) {
		std::cerr << "Warning: Failed to load RunDragon.fbx" << std::endl;
	} else {
		std::cout << "SUCCESS: RunDragon.fbx loaded" << std::endl;
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

	// AnimationPlayer 업데이트
	if (animationPlayer) {
		animationPlayer->Update(deltaTime);
	}

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
