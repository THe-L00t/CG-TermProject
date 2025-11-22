#include "Engine.h"
#include "Window.h"
#include "Renderer.h"

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
	r = std::make_unique<Renderer>();
	r->Init();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	w->onResize = [this](int w, int h) {
		r->OnWindowResize(w, h);
		};

	r->onDrawScene = [this]() {
		r->RenderTestCube();
		};

	glutDisplayFunc(Renderer::DrawScene);
	glutReshapeFunc(Window::Resize);
	glutTimerFunc(16, TimerCallback, 0);
}

void Engine::Run()
{
	
	glutMainLoop();
}

void Engine::Update()
{
	glutPostRedisplay();
}

void Engine::TimerCallback(int value)
{
	if (instance) {
		instance->Update();
	}
	glutTimerFunc(16, TimerCallback, 0);
}
