#include "window.h"

window::~window()
{
	Deactive();
}

bool window::Create()
{
	Active();
	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow(title.c_str());
	return true;
}

void window::Active()
{
	activeInstance = this;
}

void window::Deactive()
{
	if (activeInstance == this) {
		activeInstance = nullptr;
	}
}

void window::Resize(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}
