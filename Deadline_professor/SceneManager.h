#pragma once
#include "TotalHeader.h"

class Scene;
class TitleScene;
class Floor1Scene;
class Floor2Scene;
class Floor3Scene;
class TestScene;
class Player;
class Professor;
class Light;

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

	virtual void Enter() = 0;
	virtual void Exit() = 0;
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
	float fadeTimer{0.0f};
	float alpha{1.0f};
	bool fadeOut{true};
	bool keyPressed{false};
	std::unique_ptr<class Plane> titlePlane;  // 타이틀 텍스트용 Plane
	std::unique_ptr<Light> light;
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
	std::unique_ptr<class Plane> floor;
	std::unique_ptr<class Plane> ceiling;
	std::unique_ptr<class Wall> testWall;  // 테스트용 벽
	std::unique_ptr<Light> light;
};

class Floor2Scene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:
	std::unique_ptr<class Professor> professor;
	std::unique_ptr<class Plane> floor;
	std::unique_ptr<class Plane> ceiling;
	std::unique_ptr<Light> light;
};

class Floor3Scene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:
	std::unique_ptr<class Professor> professor;
	std::unique_ptr<class Plane> floor;
	std::unique_ptr<class Plane> ceiling;
	std::unique_ptr<Light> light;
};

class TestScene : public Scene
{
public:
	void Enter() override;
	void Exit() override;
	void Update(float) override;
	void Draw() override;
private:
	std::unique_ptr<Player> player;
	std::unique_ptr<Professor> lee;
	std::unique_ptr<Light> light;  // 레거시 단일 조명 (하위 호환성)
	std::vector<std::unique_ptr<Light>> lights;  // 다중 조명 시스템
	std::unique_ptr<class MapGenerator> mapGenerator;
	std::vector<std::unique_ptr<class Wall>> walls;
	std::unique_ptr<class Plane> floor;
	std::unique_ptr<class Plane> ceiling;
};