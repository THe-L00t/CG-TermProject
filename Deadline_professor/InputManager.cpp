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
	// ⭐ 매 프레임 호출되어 현재 누르고 있는 모든 키를 처리!
	if (!onceInstance) return;

	// ⭐⭐⭐ 누르고 있는 키들을 체크해서 ActionW, ActionA 등 호출
	if (onceInstance->keyStates['W'] || onceInstance->keyStates['w']) {
		if (onceInstance->ActionW) onceInstance->ActionW();
	}
	if (onceInstance->keyStates['A'] || onceInstance->keyStates['a']) {
		if (onceInstance->ActionA) onceInstance->ActionA();
	}
	if (onceInstance->keyStates['S'] || onceInstance->keyStates['s']) {
		if (onceInstance->ActionS) onceInstance->ActionS();
	}
	if (onceInstance->keyStates['D'] || onceInstance->keyStates['d']) {
		if (onceInstance->ActionD) onceInstance->ActionD();
	}
	if (onceInstance->keyStates[' ']) {
		if (onceInstance->ActionSpace) onceInstance->ActionSpace();
	}

	// ⭐ Shift 키 (왼쪽/오른쪽 모두 지원)
	if (onceInstance->shiftPressed) {
		if (onceInstance->ActionShift) onceInstance->ActionShift();
	}

	// ⭐ Ctrl 키 (왼쪽/오른쪽 모두 지원)
	if (onceInstance->ctrlPressed) {
		if (onceInstance->ActionCtrl) onceInstance->ActionCtrl();
	}
}

void InputManager::Keyboard(unsigned char key, int x, int y)
{
	if (!onceInstance || !onceInstance->cmr) return;

	// 키 눌림 상태 업데이트
	onceInstance->keyStates[key] = true;

	switch (key) {
	case'0':
		// Mouse Control Mode (토글)
		if (onceInstance->Action0) onceInstance->Action0();
		break;
	case'1':
		// Title Mode (씬 전환)
		if (onceInstance->Action1) onceInstance->Action1();
		break;
	case'2':
		// Floor 1 (씬 전환)
		if (onceInstance->Action2) onceInstance->Action2();
		break;
	case'3':
		// Floor 2 (씬 전환)
		if (onceInstance->Action3) onceInstance->Action3();
		break;
	case'4':
		// Floor 3 (씬 전환)
		if (onceInstance->Action4) onceInstance->Action4();
		break;
	case'5':
		// Test (씬 전환)
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

	// 특수 키 눌림 감지
	switch (key) {
	case GLUT_KEY_CTRL_L:
	case GLUT_KEY_CTRL_R:
		onceInstance->ctrlPressed = true;
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
	float xOffset = static_cast<float>(centerX - x);
	float yOffset = static_cast<float>(centerY - y);

	// 감도 조정
	onceInstance->cmr->Rotate(xOffset * 0.002f, yOffset * 0.002f);

	// Reset cursor to center
	glutWarpPointer(centerX, centerY);
}
