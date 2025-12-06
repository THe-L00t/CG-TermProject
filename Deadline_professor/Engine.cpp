#include "Engine.h"
#include "Window.h"
#include "Renderer.h"
#include "GameTimer.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "FBXAnimationPlayer.h"
#include "CollisionManager.h"

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
	glEnable(GL_CULL_FACE);     // 백페이스 컬링 활성화 (성능 향상)
	std::cout << "Engine: Backface culling ENABLED" << std::endl;

	gameTimer = std::make_unique<GameTimer>();

	// Camera 초기화
	camera = std::make_unique<Camera>(
		glm::vec3(0.0f, 2.0f, 5.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		45.0f,
		(float)w->GetWidth() / (float)w->GetHeight()
	);
	r->SetCamera(camera.get());
	std::cout << "Camera: Initialized" << std::endl;

	// InputManager 초기화
	inputManager = std::make_unique<InputManager>();
	inputManager->init();
	inputManager->SetCamera(camera.get());
	inputManager->SetWindow(w.get());

	// 키보드 액션 바인딩 (테스트용 - Scene에서 재설정됨)
	inputManager->ActionW = [this]() { camera->MoveForward(gameTimer->elapsedTime); };
	inputManager->ActionS = [this]() { camera->MoveBackward(gameTimer->elapsedTime); };
	inputManager->ActionA = [this]() { camera->MoveLeft(gameTimer->elapsedTime); };
	inputManager->ActionD = [this]() { camera->MoveRight(gameTimer->elapsedTime); };
	inputManager->ActionWheelUp = [this]() { camera->Zoom(1.0f); };
	inputManager->ActionWheelDown = [this]() { camera->Zoom(-1.0f); };

	// 디버깅용 씬 전환 바인딩
	inputManager->Action1 = [this]() {
		std::cout << "\n[KEY 1 PRESSED] Requesting scene change to Title" << std::endl;
		if (sceneManager) sceneManager->ChangeScene("Title");
		else std::cerr << "ERROR: SceneManager is null!" << std::endl;
	};
	inputManager->Action2 = [this]() {
		std::cout << "\n[KEY 2 PRESSED] Requesting scene change to Floor1" << std::endl;
		if (sceneManager) sceneManager->ChangeScene("Floor1");
		else std::cerr << "ERROR: SceneManager is null!" << std::endl;
	};
	inputManager->Action3 = [this]() {
		std::cout << "\n[KEY 3 PRESSED] Requesting scene change to Floor2" << std::endl;
		if (sceneManager) sceneManager->ChangeScene("Floor2");
		else std::cerr << "ERROR: SceneManager is null!" << std::endl;
	};
	inputManager->Action4 = [this]() {
		std::cout << "\n[KEY 4 PRESSED] Requesting scene change to Floor3" << std::endl;
		if (sceneManager) sceneManager->ChangeScene("Floor3");
		else std::cerr << "ERROR: SceneManager is null!" << std::endl;
	};
	inputManager->Action5 = [this]() {
		std::cout << "\n[KEY 5 PRESSED] Requesting scene change to Test" << std::endl;
		if (sceneManager) sceneManager->ChangeScene("Test");
		else std::cerr << "ERROR: SceneManager is null!" << std::endl;
	};

	// 마우스 컨트롤 토글
	inputManager->Action0 = [this]() {
		static bool mouseControlActive = false;
		mouseControlActive = !mouseControlActive;
		inputManager->SetMouseControlActive(mouseControlActive);
		std::cout << "Mouse Control: " << (mouseControlActive ? "ON" : "OFF") << std::endl;
	};

	std::cout << "InputManager: Initialized" << std::endl;

	// CollisionManager 초기화
	collisionManager = std::make_unique<CollisionManager>();
	std::cout << "CollisionManager: Initialized" << std::endl;

	// SceneManager 초기화
	sceneManager = std::make_unique<SceneManager>();
	sceneManager->ChangeScene("Title");

	// AnimationPlayer 초기화
	animationPlayer = std::make_unique<FBXAnimationPlayer>();
	const FBXModel* model = resourceManager->GetFBXModel("RunLee");
	if (model) {
		animationPlayer->Init(model);
		if (!model->animations.empty()) {
			animationPlayer->PlayAnimation(0);
			std::cout << "AnimationPlayer: Initialized with " << model->animations.size() << " animations" << std::endl;
		}
	}

	// 콜백 설정
	w->onResize = [this](int w, int h) { r->OnWindowResize(w, h); };
	r->onDrawScene = [this]() {
		if (sceneManager) {
			Scene* currentScene = sceneManager->GetCurrentScene();
			if (currentScene) currentScene->Draw();
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

	std::cout << "Engine: Initialization complete" << std::endl;
}

void Engine::LoadAssets()
{
	std::cout << "=== Loading Assets ===" << std::endl;
	std::cout << "\n--- Loading FBX/OBJ files ---" << std::endl;

	// Plane 메쉬 로드 (OBJ)
	if (!resourceManager->LoadObj("PlaneModel", "Resources/Plane.obj")) {
		std::cerr << "ERROR: Failed to load Plane.obj" << std::endl;
	} else {
		std::cout << "SUCCESS: Plane.obj loaded" << std::endl;
	}

	// Cube 메쉬 로드 (OBJ) - 벽용
	if (!resourceManager->LoadObj("CubeModel", "Resources/cube.obj")) {
		std::cerr << "ERROR: Failed to load cube.obj" << std::endl;
	} else {
		std::cout << "SUCCESS: cube.obj loaded" << std::endl;
	}

	if (!resourceManager->LoadFBX("RunLee", "Resources/RunLee.fbx")) {
		std::cerr << "ERROR: Failed to load RunLee.fbx" << std::endl;
	} else {
		std::cout << "SUCCESS: RunLee.fbx loaded" << std::endl;
	}

	if (!resourceManager->LoadFBX("RunSong", "Resources/RunSong.fbx")) {
		std::cerr << "Warning: Failed to load RunSong.fbx" << std::endl;
	} else {
		std::cout << "SUCCESS: RunSong.fbx loaded" << std::endl;
	}

	if (!resourceManager->LoadFBX("RunDragon", "Resources/RunDragon.fbx")) {
		std::cerr << "Warning: Failed to load RunDragon.fbx" << std::endl;
	} else {
		std::cout << "SUCCESS: RunDragon.fbx loaded" << std::endl;
	}

	// 텍스처 로드
	std::cout << "\n--- Loading Textures ---" << std::endl;
	std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

	if (!resourceManager->LoadTexture("RunLee", "Textures/RunLee.png")) {
		std::cerr << "Warning: Failed to load RunLee.png" << std::endl;
	} else {
		std::cout << "SUCCESS: RunLee.png loaded" << std::endl;
	}

	if (!resourceManager->LoadTexture("RunSong", "Textures/RunSong.png")) {
		std::cerr << "Warning: Failed to load RunSong.png" << std::endl;
	} else {
		std::cout << "SUCCESS: RunSong.png loaded" << std::endl;
	}

	if (!resourceManager->LoadTexture("RunDragon", "Textures/RunDragon.png")) {
		std::cerr << "Warning: Failed to load RunDragon.png" << std::endl;
	} else {
		std::cout << "SUCCESS: RunDragon.png loaded" << std::endl;
	}

	if (!resourceManager->LoadTexture("CeilingTexture", "Textures/top.png")) {
		std::cerr << "Warning: Failed to load top.png" << std::endl;
	} else {
		std::cout << "SUCCESS: top.png loaded" << std::endl;
	}

	if (!resourceManager->LoadTexture("FloorTexture", "Textures/floor.png")) {
		std::cerr << "Warning: Failed to load floor.png" << std::endl;
	} else {
		std::cout << "SUCCESS: floor.png loaded" << std::endl;
	}

	if (!resourceManager->LoadTexture("WallTexture", "Textures/wall.png")) {
		std::cerr << "Warning: Failed to load wall.png" << std::endl;
	} else {
		std::cout << "SUCCESS: wall.png loaded" << std::endl;
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
