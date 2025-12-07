#include "InputManager.h"
#include "Window.h"
#include "Camera.h"

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

void InputManager::UpdateKeyStates(float deltaTime)
{
	if (!onceInstance || !onceInstance->cmr) return;

	// 현재 누르고 있는 키들에 따라 카메라 이동
	if (onceInstance->keyStates['W'] || onceInstance->keyStates['w']) {
		onceInstance->cmr->MoveForward(deltaTime);
	}
	if (onceInstance->keyStates['A'] || onceInstance->keyStates['a']) {
		onceInstance->cmr->MoveLeft(deltaTime);
	}
	if (onceInstance->keyStates['S'] || onceInstance->keyStates['s']) {
		onceInstance->cmr->MoveBackward(deltaTime);
	}
	if (onceInstance->keyStates['D'] || onceInstance->keyStates['d']) {
		onceInstance->cmr->MoveRight(deltaTime);
	}
	if (onceInstance->keyStates[' ']) { // Space key
		onceInstance->cmr->MoveUp(deltaTime);
	}
	if (onceInstance->ctrlPressed) { // Ctrl key
		onceInstance->cmr->MoveDown(deltaTime);
	}
}

void InputManager::Keyboard(unsigned char key, int x, int y)
{
	if (!onceInstance || !onceInstance->cmr) return;

	// 키 눌림 상태 업데이트
	onceInstance->keyStates[key] = true;

	switch (key) {
	case'W':case'w':
		if (onceInstance->ActionW) onceInstance->ActionW();
		break;
	case'A':case'a':
		if (onceInstance->ActionA) onceInstance->ActionA();
		break;
	case'S':case's':
		if (onceInstance->ActionS) onceInstance->ActionS();
		break;
	case'D':case'd':
		if (onceInstance->ActionD) onceInstance->ActionD();
		break;
	case ' ':
		if (onceInstance->ActionSpace) onceInstance->ActionSpace();
		break;
	case'0':
		// Mouse Control Mode
		if (onceInstance->Action0) onceInstance->Action0();
		break;
	case'1':
		// Title Mode
		if (onceInstance->Action1) onceInstance->Action1();
		break;
	case'2':
		// Floor 1
		if (onceInstance->Action2) onceInstance->Action2();
		break;
	case'3':
		// Floor 2
		if (onceInstance->Action3) onceInstance->Action3();
		break;
	case'4':
		// Floor 3
		if (onceInstance->Action4) onceInstance->Action4();
		break;
	case'5':
		// Test
		if (onceInstance->Action5) onceInstance->Action5();
		break;
	case 27: // ESC key
		exit(0);
		break;
	}
}

void InputManager::KeyboardUp(unsigned char key, int x, int y)
{
	if (!onceInstance) return;

	// 키가 릴리스 상태 업데이트
	onceInstance->keyStates[key] = false;
}


void InputManager::SKeyboard(int key, int x, int y)
{
	if (!onceInstance) return;

	// 특수 키 누림 감지
	// GLUT_KEY_CTRL_L = 114, GLUT_KEY_CTRL_R = 115
	// GLUT_KEY_SHIFT_L = 112, GLUT_KEY_SHIFT_R = 113
	// GLUT_KEY_ALT_L = 116, GLUT_KEY_ALT_R = 117

	switch (key) {
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		onceInstance->ctrlPressed = true;
		if (onceInstance->ActionCtrl) onceInstance->ActionCtrl();
		break;
	case GLUT_KEY_SHIFT_L:
	case GLUT_KEY_SHIFT_R:
		onceInstance->shiftPressed = true;
		break;
	case GLUT_KEY_ALT_L:
	case GLUT_KEY_ALT_R:
		onceInstance->altPressed = true;
		break;
	}
}

void InputManager::SKeyboardUp(int key, int x, int y)
{
	if (!onceInstance) return;

	// 특수 키 릴리스 감지
	switch (key) {
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		onceInstance->ctrlPressed = false;
		break;
	case GLUT_KEY_SHIFT_L:
	case GLUT_KEY_SHIFT_R:
		onceInstance->shiftPressed = false;
		break;
	case GLUT_KEY_ALT_L:
	case GLUT_KEY_ALT_R:
		onceInstance->altPressed = false;
		break;
	}
}
void InputManager::Mouse(int button, int state, int x, int y)
{
	if (!onceInstance) return;

	if (button == 3 && state == GLUT_DOWN) { // Mouse wheel up
		if (onceInstance->ActionWheelUp) onceInstance->ActionWheelUp();
	}
	if (button == 4 && state == GLUT_DOWN) { // Mouse wheel down
		if (onceInstance->ActionWheelDown) onceInstance->ActionWheelDown();
	}
}

void InputManager::PassiveMotion(int x, int y)
{
	if (!onceInstance || !onceInstance->cmr || !onceInstance->mouseControl || !onceInstance->window)
		return;

	static bool firstMouse = true;
	static int lastX = 0;
	static int lastY = 0;

	int centerX = onceInstance->window->GetWidth() / 2;
	int centerY = onceInstance->window->GetHeight() / 2;

	if (firstMouse) {
		lastX = centerX;
		lastY = centerY;
		firstMouse = false;
	}

	// Calculate mouse movement delta
	float xOffset = static_cast<float>(centerX - x); // Reversed for inverted control
	float yOffset = static_cast<float>(centerY - y); // Reversed: y-coordinates go from bottom to top

	// Rotate camera based on mouse movement
	onceInstance->cmr->Rotate(xOffset * 0.01f, yOffset * 0.01f);

	// Reset cursor to center
	glutWarpPointer(centerX, centerY);
}
