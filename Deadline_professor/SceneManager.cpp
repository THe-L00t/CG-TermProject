#include "SceneManager.h"

SceneManager::SceneManager()
{
	ChangeScene("Title");
}

void SceneManager::update(float)
{
	if (currentScene) {
		// 씬 업데이트 함수 호출
	}
}

void SceneManager::ChangeScene(const std::string& sceneName)
{
	auto it = sceneFactory.find(sceneName);
	if (it not_eq sceneFactory.end()) {
		if (currentScene) {
			//씬 exit함수 호출
		}
		currentScene = it->second();
		//씬 업데이트 함수 호출
	}
}

Scene* SceneManager::GetCurrentScene() const
{
	return currentScene.get();
}
