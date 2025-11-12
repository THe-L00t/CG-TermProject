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
	w = std::make_unique<window>{ window() };
}

void Engine::Run()
{
}

void Engine::Update()
{
}
