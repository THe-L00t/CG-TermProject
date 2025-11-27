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
		glutWarpPointer();	// 윈도우 객체에서 창 크기 받아오기
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
