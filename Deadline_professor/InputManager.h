#pragma once
#include "TotalHeader.h"

class Camera;
class Window;

class InputManager
{
public:
	void init();
	void SetCamera(Camera* camera);
	void SetWindow(Window* window);
	void SetMouseControlActive(bool);

	//콜백용 함수
	static void Keyboard(unsigned char, int, int);
	static void SKeyboard(int, int, int);
	static void Mouse(int, int, int, int);
	static void PassiveMotion(int, int);

	// 키 할당 함수 객체
	std::function<void()> ActionW;
	std::function<void()> ActionA;
	std::function<void()> ActionS;
	std::function<void()> ActionD;
	std::function<void()> ActionSpace;  // 위로 이동 (자유 비행)
	std::function<void()> ActionShift;  // 아래로 이동 (자유 비행)
	std::function<void()> ActionWheelUp;
	std::function<void()> ActionWheelDown;

	// 디버깅용 씬 전환 함수 객체
	std::function<void()> Action1; // Title Scene
	std::function<void()> Action2; // Floor1 Scene
	std::function<void()> Action3; // Floor2 Scene
	std::function<void()> Action4; // Floor3 Scene
	std::function<void()> Action5; // Test Scene
	std::function<void()> Action0; // Mouse Control Toggle

private:
	static InputManager* onceInstance;
	Camera* cmr;
	Window* window;
	bool mouseControl{ false };
};

