#pragma once

class Camera
class InputManager
{
public:
	void init();

	void SetMouseControlActive(bool);

	//콜백용 함수 
	static void Keyboard(unsigned char, int, int);
	static void SKeyboard(int, int, int);
	static void Mouse(int, int, int, int);
private:
	static InputManager* onceInstance;
	Camera* c;
	bool mouseControl{ false };
};

