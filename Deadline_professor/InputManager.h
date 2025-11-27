#pragma once
#include "TotalHeader.h"

class Camera;

class InputManager
{
public:
	void init();

	void SetMouseControlActive(bool);

	//콜백용 함수 
	static void Keyboard(unsigned char, int, int);
	static void SKeyboard(int, int, int);
	static void Mouse(int, int, int, int);

	// 키 할당 함수 객체
	std::function<void()> ActionW;
	std::function<void()> ActionA;
	std::function<void()> ActionS;
	std::function<void()> ActionD;
	std::function<void()> ActionWheelUp;
	std::function<void()> ActionWheelDown;

private:
	static InputManager* onceInstance;
	Camera* cmr;
	bool mouseControl{ false };
};

