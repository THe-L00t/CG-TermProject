#include "Window.h"
#include <iostream>

Window* Window::activeInstance = nullptr;

Window::~Window()
{
	Deactive();
}

bool Window::Create()
{
	Active();

	// OpenGL 3.3 코어 프로파일 요청
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow(title.c_str());

	std::cout << "Window: OpenGL 3.3 Core Profile requested" << std::endl;
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


