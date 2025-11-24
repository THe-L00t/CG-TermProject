#include "Renderer.h"
Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	Deactive();
}

void Renderer::Init()
{
	Active();
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

void Renderer::OnWindowResize(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Renderer::DrawScene(GLvoid)
{
	if (activeInstance->onDrawScene) {
		activeInstance->onDrawScene();
	}
		
}

