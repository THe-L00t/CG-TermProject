#pragma once
#include "TotalHeader.h"

class Window
{
public:
	Window() = default;
	~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

	bool Create();
	void Active();
	void Deactive();
	int GetWidth() const;
	int GetHeight() const;

	// 콜백용 함수
	std::function<void(int, int)> onResize;
	static void Resize(int, int);

private:
	static Window* activeInstance;

	int height{ 1080 };
	int width{ 1920 };
	std::string title{ "Deadline:교수님" };

	int pWindowH{};
	int pWindowW{};
	int pWindowX{};
	int pWindowY{};

};

