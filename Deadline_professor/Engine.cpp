#include "Engine.h"
#include "Window.h"
#include "Renderer.h"

Engine::Engine()
{
}

Engine::~Engine()
{
}

void Engine::Initialize(int argc, char** argv)
{
	glutInit(&argc, argv);
	w = std::make_unique<Window>();
	r = std::make_unique<Renderer>();
	
	w->Create();

	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else {
		std::cout << "GLEW Initialized\n";
	}

	r->Init();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	w->onResize = [this](int w, int h) {
		r->OnWindowResize(w, h);
		};

	//glutDisplayFunc(drawScene);
	glutReshapeFunc(w->Resize);
	/*glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(PassiveMotion);
	glutTimerFunc(1, loop, 1);*/
}

void Engine::Run()
{
	
	glutMainLoop();
}

void Engine::Update()
{
}
