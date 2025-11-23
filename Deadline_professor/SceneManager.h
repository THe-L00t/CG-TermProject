#pragma once
#include "TotalHeader.h"

class Scene;
class GameScene;
class TitleScene;
class TestScene;

class SceneManager
{
public:
	SceneManager();
	~SceneManager() = default;

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	void update(float);
	void ChangeScene(const std::string&);
	Scene* GetCurrentScene() const;

private:
	std::unique_ptr<Scene> currentScene;
	std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>> sceneFactory;
};

class Scene
{
public:
	virtual ~Scene() = default;

	virtual void Enter();
	virtual void Exit();
	virtual void Update(float) = 0;
	virtual void Draw() = 0;
private:

};