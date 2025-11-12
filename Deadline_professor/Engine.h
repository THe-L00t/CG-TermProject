#pragma once
#include "TotalHeader.h"

class window;

class Engine
{
public:
	Engine();
	~Engine();

	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	void Initialize(int, char**);
	void Run();
	void Update();

private:
	static Engine* instance;


	std::unique_ptr<window> w;
};

