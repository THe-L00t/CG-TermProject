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

//---------------------------------------------------------------Floor1Scene

void Floor1Scene::Enter()
{
}

void Floor1Scene::Exit()
{
}

void Floor1Scene::Update(float)
{
}

void Floor1Scene::Draw()
{
}

//---------------------------------------------------------------Floor2Scene

void Floor2Scene::Enter()
{
}

void Floor2Scene::Exit()
{
}

void Floor2Scene::Update(float)
{
}

void Floor2Scene::Draw()
{
}

//---------------------------------------------------------------Floor3Scene

void Floor3Scene::Enter()
{
}

void Floor3Scene::Exit()
{
}

void Floor3Scene::Update(float)
{
}

void Floor3Scene::Draw()
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
