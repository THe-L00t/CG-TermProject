#include "SceneManager.h"

SceneManager::SceneManager()
{
	ChangeScene("Title");
}

void SceneManager::update(float deltaTime)
{
	if (currentScene) {
		currentScene->Update(deltaTime);
	}
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	auto it = sceneFactory.find(sceneName);
	if (it not_eq sceneFactory.end()) {
		if (currentScene) {
			currentScene->Exit();
		}
		currentScene = it->second();
		currentScene->Enter();
	}
}

Scene* SceneManager::GetCurrentScene() const
{
	return currentScene.get();
}

//---------------------------------------------------------------------Scene

void Scene::Enter()
{
}

void Scene::Exit()
{
}

//-----------------------------------------------------------------TitleScene

void TitleScene::Enter()
{
}

void TitleScene::Exit()
{
}

void TitleScene::Update(float)
{
}

void TitleScene::Draw()
{
}

//-----------------------------------------------------------------GameScene

void GameScene::Enter()
{
}

void GameScene::Exit()
{
}

void GameScene::Update(float)
{
}

void GameScene::Draw()
{
}

//-----------------------------------------------------------------TestScene

void TestScene::Enter()
{
}

void TestScene::Exit()
{
}

void TestScene::Update(float)
{
}

void TestScene::Draw()
{
}
