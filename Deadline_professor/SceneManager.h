#pragma once
#include "TotalHeader.h"

class Scene;
class TitleScene;
class Floor1Scene;
class Floor2Scene;
class Floor3Scene;
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

class TitleScene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:

};

class Floor1Scene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:
	std::unique_ptr<class Player> player;
	std::unique_ptr<class Professor> professor;
};

class Floor2Scene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:

};

class Floor3Scene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:

};

class TestScene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:

};