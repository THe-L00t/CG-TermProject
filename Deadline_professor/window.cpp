#include "Window.h"

Window* Window::activeInstance = nullptr;

Window::~Window()
{
	Deactive();
}

bool Window::Create()
{
	Active();
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow(title.c_str());
	return true;
}

void Window::Active()
{
	activeInstance = this;
}

void Window::Deactive()
{
	if (activeInstance == this) {
		activeInstance = nullptr;
	}
}

int Window::GetWidth() const
{
	return width;
}

int Window::GetHeight() const
{
	return height;
}

void Window::Resize(int w, int h)
{
	activeInstance->width = w;
	activeInstance->height = h;

	if (activeInstance->onResize) {
		activeInstance->onResize(w, h);
	}
}


