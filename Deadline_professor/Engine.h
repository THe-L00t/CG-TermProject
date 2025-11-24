#pragma once
#include "TotalHeader.h"

class Window;
class Renderer;
class GameTimer;
class ResourceManager;

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

	ResourceManager* GetResourceManager() const { return resourceManager.get(); }

	static void TimerCallback(int value);

private:
	void LoadAssets();

	static Engine* instance;

	std::unique_ptr<Window> w;
	std::unique_ptr<Renderer> r;
	std::unique_ptr<GameTimer> gameTimer;
	std::unique_ptr<ResourceManager> resourceManager;
};

