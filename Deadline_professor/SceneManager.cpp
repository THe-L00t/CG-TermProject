#include "SceneManager.h"
#include "Engine.h"
#include "Renderer.h"
#include "ResourceManager.h"

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
	std::cout << "=== TestScene Entered ===" << std::endl;
	std::cout << "TestScene: Game Framework is running!" << std::endl;

	// FBX 모델 확인
	extern Engine* g_engine;
	if (g_engine) {
		ResourceManager* resMgr = g_engine->GetResourceManager();

		if (!resMgr) {
			std::cerr << "TestScene: ResourceManager is null" << std::endl;
			return;
		}

		const FBXModel* model = resMgr->GetFBXModel("RunLee");

		if (model) {
			std::cout << "TestScene: RunLee FBX model found with " << model->meshes.size() << " meshes" << std::endl;
		}
		else {
			std::cout << "TestScene: RunLee FBX model not found" << std::endl;
		}
	}
	else {
		std::cerr << "TestScene: g_engine is null" << std::endl;
	}
}

void TestScene::Exit()
{
	std::cout << "=== TestScene Exited ===" << std::endl;
}

void TestScene::Update(float deltaTime)
{
	// 5초마다 한 번씩 로그 출력
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
