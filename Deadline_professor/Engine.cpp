#include "Engine.h"
#include "Window.h"
#include "Renderer.h"
#include "GameTimer.h"
#include "ResourceManager.h"

Engine* Engine::instance = nullptr;

Engine::Engine()
{
	instance = this;
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

	w->onResize = [this](int w, int h) {
		r->OnWindowResize(w, h);
		};

	r->onDrawScene = [this]() {
		r->RenderTestCube();
		};

	glutDisplayFunc(Renderer::DrawScene);
	glutReshapeFunc(Window::Resize);
	glutTimerFunc(1, TimerCallback, 0);
}

void Engine::LoadAssets()
{
	std::cout << "=== Loading Assets ===" << std::endl;

	// OBJ 파일 로드
	resourceManager->LoadObj("bugatti", "bugatti.obj");

	std::cout << "=== Assets Loaded ===" << std::endl;
}

void Engine::Run()
{
	
	glutMainLoop();
}

void Engine::Update()
{
	gameTimer->Update();
	glutPostRedisplay();
}

void Engine::TimerCallback(int value)
{
	if (instance) {
		instance->Update();
		glutTimerFunc(1, TimerCallback, 0);
	}
}
