#pragma once
#include "TotalHeader.h"

class window
{
public:
	window() = default;
	~window();

	window(const window&) = delete;
	window& operator=(const window&) = delete;

	bool Create();
	void Active();
	void Deactive();

	// 콜백용 함수 
	void Resize(int, int);

private:
	static window* activeInstance;

	int height{ 1080 };
	int width{ 1920 };
	std::string title{ "Deadline:교수님" };

	int pWindowH{};
	int pWindowW{};
	int pWindowX{};
	int pWindowY{};

};

