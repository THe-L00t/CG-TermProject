#include "Renderer.h"
Renderer::Renderer()
{

}

void Renderer::SetViewport(int x, int y, int w, int h)
{
	glViewport(x, y, w, h);
}

void Renderer::OnWindowResize(int w, int h)
{
	SetViewport(0, 0, w, h);
}
