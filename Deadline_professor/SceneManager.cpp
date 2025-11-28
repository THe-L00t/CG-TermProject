#include "SceneManager.h"
#include "Engine.h"
#include "AnimationPlayer.h"
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

	// RunLee 메시 설정
	extern Engine* g_engine;
	if (g_engine) {
		AnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		ResourceManager* resMgr = g_engine->GetResourceManager();

		if (!animPlayer || !resMgr) {
			std::cerr << "TestScene: AnimationPlayer or ResourceManager is null" << std::endl;
			return;
		}

		const XMeshData* meshData = resMgr->GetXMeshData("RunLee");

		if (meshData && animPlayer) {
			std::cout << "TestScene: RunLee mesh found" << std::endl;
			animPlayer->SetMesh(meshData);

			// 애니메이션이 있으면 첫 번째 애니메이션 재생
			if (!meshData->animations.empty()) {
				animPlayer->PlayAnimation(0, true);
				std::cout << "TestScene: Playing animation: " << meshData->animations[0].name << std::endl;
			}
			else {
				std::cout << "TestScene: No animations found in RunLee mesh" << std::endl;
			}
		}
		else {
			std::cout << "TestScene: RunLee mesh not found, using default cube" << std::endl;
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
	// AnimationPlayer 업데이트
	extern Engine* g_engine;
	if (g_engine) {
		AnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		if (animPlayer) {
			if (animPlayer->IsPlaying()) {
				animPlayer->Update(deltaTime);
			}
		}
	}

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
