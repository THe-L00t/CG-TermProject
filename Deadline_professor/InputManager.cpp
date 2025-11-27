#include "InputManager.h"

InputManager* InputManager::onceInstance = nullptr;

void InputManager::init()
{
	onceInstance = this;
}

void InputManager::SetMouseControlActive(bool active)
{
	mouseControl = active;
	if (active) {
		glutSetCursor(GLUT_CURSOR_NONE);
	}
}

void InputManager::Keyboard(unsigned char, int, int)
{
}

void InputManager::SKeyboard(int, int, int)
{
}

void InputManager::Mouse(int, int, int, int)
{
}
