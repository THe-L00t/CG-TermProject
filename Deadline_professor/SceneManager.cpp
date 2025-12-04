#include "SceneManager.h"
#include "Engine.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Player.h"
#include "Professor.h"
#include "Camera.h"
#include "InputManager.h"
#include "GameTimer.h"

SceneManager::SceneManager()
{
	// Scene Factory 초기화
	sceneFactory["Title"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<TitleScene>();
	};
	sceneFactory["Floor1"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<Floor1Scene>();
	};
	sceneFactory["Floor2"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<Floor2Scene>();
	};
	sceneFactory["Floor3"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<Floor3Scene>();
	};
	sceneFactory["Test"] = []() -> std::unique_ptr<Scene> {
		return std::make_unique<TestScene>();
	};

	std::cout << "SceneManager: Scene Factory Initialized (Title, Floor1-3, Test)" << std::endl;
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
	std::cout << "=== Floor1Scene Entered ===" << std::endl;

	extern Engine* g_engine;
	if (!g_engine) {
		std::cerr << "Floor1Scene: g_engine is null" << std::endl;
		return;
	}

	Camera* camera = g_engine->GetCamera();
	InputManager* inputMgr = g_engine->GetInputManager();
	GameTimer* timer = g_engine->GetGameTimer();

	// Player 생성 및 초기화
	player = std::make_unique<Player>();
	player->Init(camera);
	player->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	player->SetResourceID("PlayerModel"); // 나중에 실제 모델 지정
	std::cout << "Floor1Scene: Player created at origin" << std::endl;

	// Professor 생성 및 초기화
	professor = std::make_unique<Professor>("RunDragon", "RunAnimation");
	professor->SetPosition(glm::vec3(5.0f, 0.0f, 0.0f));
	professor->SetPlayerReference(player.get());
	std::cout << "Floor1Scene: Professor created at (5, 0, 0)" << std::endl;

	// InputManager의 액션을 Player 이동에 연결
	if (inputMgr && timer) {
		inputMgr->ActionW = [this, timer]() {
			if (player) {
				player->MoveForward(timer->elapsedTime);
			}
		};
		inputMgr->ActionS = [this, timer]() {
			if (player) {
				player->MoveBackward(timer->elapsedTime);
			}
		};
		inputMgr->ActionA = [this, timer]() {
			if (player) {
				player->MoveLeft(timer->elapsedTime);
			}
		};
		inputMgr->ActionD = [this, timer]() {
			if (player) {
				player->MoveRight(timer->elapsedTime);
			}
		};
		std::cout << "Floor1Scene: Input actions bound to Player" << std::endl;
	}
}

void Floor1Scene::Exit()
{
	std::cout << "=== Floor1Scene Exited ===" << std::endl;

	// InputManager 액션 해제
	extern Engine* g_engine;
	if (g_engine) {
		InputManager* inputMgr = g_engine->GetInputManager();
		if (inputMgr) {
			inputMgr->ActionW = nullptr;
			inputMgr->ActionS = nullptr;
			inputMgr->ActionA = nullptr;
			inputMgr->ActionD = nullptr;
		}
	}

	player.reset();
	professor.reset();
}

void Floor1Scene::Update(float deltaTime)
{
	if (player) {
		player->Update(deltaTime);
	}
	if (professor) {
		professor->Update(deltaTime);
	}
}

void Floor1Scene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	if (!renderer) return;

	// Professor 렌더링 (FBX 모델 사용)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();
		renderer->RenderFBXModel(professor->GetMeshKey(), professorMatrix);
	}

	// Player 렌더링 (나중에 모델 추가 시)
	// if (player && player->IsActive()) {
	//     glm::mat4 playerMatrix = player->GetModelMat();
	//     renderer->RenderFBXModel(player->GetResourceID(), playerMatrix);
	// }
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
	// Engine을 통해 Renderer와 리소스에 접근
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	ResourceManager* resMgr = g_engine->GetResourceManager();
	FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();

	if (!renderer || !resMgr) return;

	// FBX 모델 렌더링
	const FBXModel* model = resMgr->GetFBXModel("RunDragon");
	if (model) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));

		// 애니메이션이 있으면 애니메이션과 함께 렌더링
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXModelWithAnimation("RunDragon", modelMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBXModel("RunDragon", modelMatrix);
		}
	}
}
