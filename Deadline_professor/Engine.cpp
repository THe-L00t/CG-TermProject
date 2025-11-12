#include "Engine.h"
#include "window.h"

Engine::Engine()
{
}

Engine::~Engine()
{
}

void Engine::Initialize(int argc, char** argv)
{
	glutInit(&argc, argv);
	w = std::make_unique<window>();
	w->Create();

	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

void Engine::Run()
{
}

void Engine::Update()
{
}
