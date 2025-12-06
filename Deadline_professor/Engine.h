#pragma once
#include "TotalHeader.h"

class Window;
class Renderer;
class GameTimer;
class ResourceManager;
class Camera;
class InputManager;
class SceneManager;
class FBXAnimationPlayer;
class CollisionManager;

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
	Renderer* GetRenderer() const { return r.get(); }
	Camera* GetCamera() const { return camera.get(); }
	FBXAnimationPlayer* GetAnimationPlayer() const { return animationPlayer.get(); }
	InputManager* GetInputManager() const { return inputManager.get(); }
	GameTimer* GetGameTimer() const { return gameTimer.get(); }
	SceneManager* GetSceneManager() const { return sceneManager.get(); }
	CollisionManager* GetCollisionManager() const { return collisionManager.get(); }

	static void TimerCallback(int value);

private:
	void LoadAssets();

	static Engine* instance;

	std::unique_ptr<Window> w;
	std::unique_ptr<Renderer> r;
	std::unique_ptr<GameTimer> gameTimer;
	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<Camera> camera;
	std::unique_ptr<InputManager> inputManager;
	std::unique_ptr<SceneManager> sceneManager;
	std::unique_ptr<FBXAnimationPlayer> animationPlayer;
	std::unique_ptr<CollisionManager> collisionManager;
};

// 전역 엔진 포인터 (Scene에서 접근용)
extern Engine* g_engine;

