#pragma once
#include "TotalHeader.h"

class Window;
class Renderer;

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

	static void UpdateCallback(int);

private:
	static Engine* instance;

	std::unique_ptr<Window> w;
	std::unique_ptr<Renderer> r;
};

