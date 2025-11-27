#include "InputManager.h"
#include "Window.h"

InputManager* InputManager::onceInstance = nullptr;

void InputManager::init()
{
	onceInstance = this;
}

void InputManager::SetCamera(Camera* camera)
{
	cmr = camera;
}

void InputManager::SetWindow(Window* win)
{
	window = win;
}

void InputManager::SetMouseControlActive(bool active)
{
	mouseControl = active;
	if (active) {
		glutSetCursor(GLUT_CURSOR_NONE);
		// 윈도우 중앙으로 마우스 커서 이동
		if (window) {
			int centerX = window->GetWidth() / 2;
			int centerY = window->GetHeight() / 2;
			glutWarpPointer(centerX, centerY);
		}
	}
	else {
		glutSetCursor(GLUT_CURSOR_INHERIT);
	}

}

void InputManager::Keyboard(unsigned char key, int x, int y)
{
	if (not onceInstance->cmr) return;
	float distance; //= onceInstance->cmr->이동속도 출력 함수 * deltaTime;
	switch (key) {
	case'W':case'w':
		if (not onceInstance->ActionW) onceInstance->ActionW();
		break;
	case'A':case'a':
		if (not onceInstance->ActionA) onceInstance->ActionA();
		break;
	case'S':case's':
		if (not onceInstance->ActionS) onceInstance->ActionS();
		break;
	case'D':case'd':
		if (not onceInstance->ActionD) onceInstance->ActionD();
	case'1':
		onceInstance->SetMouseControlActive(true);
		break;
	}

}

void InputManager::SKeyboard(int, int, int)
{
}

void InputManager::Mouse(int button, int state, int x, int y)
{
	if (button == 3) {
		onceInstance->ActionWheelUp();
	}
	if (button == 4) {
		onceInstance->ActionWheelDown();
	}
	
}
