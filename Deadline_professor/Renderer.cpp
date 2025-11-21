#include "Renderer.h"
Renderer::Renderer()
{

}

void Renderer::OnWindowResize(int w, int h)
{
	glViewport(0, 0, w, h);
}
