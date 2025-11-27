#include "SceneManager.h"

SceneManager::SceneManager()
{
	// Scene Factory 초기화
	sceneFactory["Title"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<TitleScene>();
	};
	sceneFactory["Game"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<GameScene>();
	};
	sceneFactory["Test"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<TestScene>();
	};

	std::cout << "SceneManager: Scene Factory Initialized (Title, Game, Test)" << std::endl;
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
		std::cout << "SceneManager: Changed to " << sceneName << " scene" << std::endl;
	}
	else {
		std::cerr << "SceneManager: Scene '" << sceneName << "' not found!" << std::endl;
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
	std::cout << "=== TestScene Entered ===" << std::endl;
	std::cout << "TestScene: Game Framework is running!" << std::endl;
}

void TestScene::Exit()
{
	std::cout << "=== TestScene Exited ===" << std::endl;
}

void TestScene::Update(float deltaTime)
{
	// 5초마다 한 번씩 로그 출력 (프레임 카운트 기반)
	static float accumulatedTime = 0.0f;
	accumulatedTime += deltaTime;

	if (accumulatedTime >= 5.0f) {
		std::cout << "TestScene: Update running... (deltaTime: " << deltaTime << "s)" << std::endl;
		accumulatedTime = 0.0f;
	}
}

void TestScene::Draw()
{
	// TestScene에서는 별도 렌더링 없음 (Renderer가 이미 처리)
}
