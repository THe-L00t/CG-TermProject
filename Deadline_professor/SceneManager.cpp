#include "SceneManager.h"
#include "Engine.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Player.h"
#include "Professor.h"
#include "Light.h"
#include "Camera.h"
#include "InputManager.h"
#include "GameTimer.h"
#include "FBXAnimationPlayer.h"
#include "Plane.h"

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

	std::cout << "SceneManager: Initialized" << std::endl;
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
	std::cout << "TitleScene: Entered" << std::endl;
	fadeTimer = 0.0f;
	alpha = 1.0f;
	fadeOut = true;
	keyPressed = false;

	// InputManager 액션 바인딩 (멤버 변수 사용)
	extern Engine* g_engine;
	if (g_engine) {
		InputManager* inputMgr = g_engine->GetInputManager();
		if (inputMgr) {
			inputMgr->ActionW = [this]() { keyPressed = true; };
			inputMgr->ActionA = [this]() { keyPressed = true; };
			inputMgr->ActionS = [this]() { keyPressed = true; };
			inputMgr->ActionD = [this]() { keyPressed = true; };
		}
	}
}

void TitleScene::Exit()
{
	std::cout << "TitleScene: Exited" << std::endl;

	// InputManager 액션 해제
	extern Engine* g_engine;
	if (g_engine) {
		InputManager* inputMgr = g_engine->GetInputManager();
		if (inputMgr) {
			inputMgr->ActionW = nullptr;
			inputMgr->ActionA = nullptr;
			inputMgr->ActionS = nullptr;
			inputMgr->ActionD = nullptr;
		}
	}
}

void TitleScene::Update(float deltaTime)
{
	// 페이드 효과 (1초 주기: 0.5초 페이드아웃 + 0.5초 페이드인)
	const float fadeDuration = 1.0f;
	const float fadeSpeed = 2.0f; // 1초에 1.0 변화 (0.0 ~ 1.0)

	fadeTimer += deltaTime;

	if (fadeOut) {
		// 페이드 아웃 (1.0 -> 0.0)
		alpha -= deltaTime * fadeSpeed;
		if (alpha <= 0.0f) {
			alpha = 0.0f;
			fadeOut = false;
			fadeTimer = 0.0f;
		}
	} else {
		// 페이드 인 (0.0 -> 1.0)
		alpha += deltaTime * fadeSpeed;
		if (alpha >= 1.0f) {
			alpha = 1.0f;
			fadeOut = true;
			fadeTimer = 0.0f;
		}
	}

	// 키 입력 시 Floor1Scene으로 전환
	if (keyPressed) {
		extern Engine* g_engine;
		if (g_engine) {
			SceneManager* sceneMgr = g_engine->GetSceneManager();
			if (sceneMgr) {
				sceneMgr->ChangeScene("Floor1");
			}
		}
		keyPressed = false;
	}
}

void TitleScene::Draw()
{
	// 2D 오버레이 모드로 전환
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	extern Engine* g_engine;
	if (g_engine) {
		// 윈도우 크기 가져오기 (기본값 1920x1080)
		gluOrtho2D(0, 1920, 0, 1080);
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// 깊이 테스트 비활성화
	glDisable(GL_DEPTH_TEST);

	// 블렌딩 활성화 (알파 블렌딩)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 텍스트 색상 (흰색 + 알파값)
	glColor4f(1.0f, 1.0f, 1.0f, alpha);

	// 텍스트 출력 위치 계산 (중앙 하단)
	const char* text = "PRESS ANY KEY TO START";
	int textLength = strlen(text);

	float x = 1920 / 2.0f - (textLength * 13.0f) / 2.0f; // 중앙 정렬 (TIMES_ROMAN_24는 약 13픽셀 너비)
	float y = 300.0f; // 하단에서 300픽셀 위

	glRasterPos2f(x, y);

	// 큰 폰트 사용 (GLUT_BITMAP_TIMES_ROMAN_24)
	for (int i = 0; i < textLength; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
	}

	// 블렌딩 비활성화
	glDisable(GL_BLEND);

	// 깊이 테스트 다시 활성화
	glEnable(GL_DEPTH_TEST);

	// 행렬 복원
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

//---------------------------------------------------------------Floor1Scene

void Floor1Scene::Enter()
{
	std::cout << "Floor1Scene: Entered" << std::endl;

	extern Engine* g_engine;
	if (!g_engine) {
		std::cerr << "ERROR: Floor1Scene - g_engine is null" << std::endl;
		return;
	}

	Camera* camera = g_engine->GetCamera();
	InputManager* inputMgr = g_engine->GetInputManager();
	GameTimer* timer = g_engine->GetGameTimer();

	// Player 생성 및 초기화
	player = std::make_unique<Player>();
	player->Init(camera);
	player->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	player->SetResourceID("PlayerModel");

	// Professor 생성 및 초기화 (1층: RunSong)
	professor = std::make_unique<Professor>("RunSong", "RunAnimation", 0.6f, 1.8f, 0.6f);
	professor->SetPosition(glm::vec3(5.0f, 0.0f, 0.0f));
	professor->SetPlayerReference(player.get());

	// AnimationPlayer를 RunSong 모델로 초기화
	FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
	ResourceManager* resMgr = g_engine->GetResourceManager();
	if (animPlayer && resMgr) {
		const FBXModel* model = resMgr->GetFBXModel("RunSong");
		if (model) {
			animPlayer->Init(model);
			if (!model->animations.empty()) {
				animPlayer->PlayAnimation(0);
				std::cout << "Floor1Scene: RunSong animation initialized" << std::endl;
			}
		}
	}

	// 바닥 생성 및 초기화
	floor = std::make_unique<Plane>();
	floor->SetOrientation(Plane::Orientation::UP);
	floor->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f)); // 바닥을 y=-1 위치에 배치
	floor->SetSize(50.0f, 50.0f); // 50x50 크기의 바닥
	floor->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	floor->SetColor(glm::vec3(0.9f, 0.9f, 0.9f)); // 밝은 회색

	// 천장 생성 및 초기화
	ceiling = std::make_unique<Plane>();
	ceiling->SetOrientation(Plane::Orientation::DOWN);
	ceiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f)); // 천장을 y=5 위치에 배치
	ceiling->SetSize(50.0f, 50.0f); // 50x50 크기의 천장
	ceiling->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	ceiling->SetColor(glm::vec3(0.9f, 0.9f, 0.9f)); // 밝은 회색

	// InputManager 액션을 Player 이동에 연결
	if (inputMgr && timer) {
		inputMgr->ActionW = [this, timer]() { if (player) player->MoveForward(timer->elapsedTime); };
		inputMgr->ActionS = [this, timer]() { if (player) player->MoveBackward(timer->elapsedTime); };
		inputMgr->ActionA = [this, timer]() { if (player) player->MoveLeft(timer->elapsedTime); };
		inputMgr->ActionD = [this, timer]() { if (player) player->MoveRight(timer->elapsedTime); };
	}

	std::cout << "Floor1Scene: Player and Professor initialized" << std::endl;
}

void Floor1Scene::Exit()
{
	std::cout << "Floor1Scene: Exited" << std::endl;

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
	floor.reset();
	ceiling.reset();
}

void Floor1Scene::Update(float deltaTime)
{
	if (player) {
		player->Update(deltaTime);
	}
	if (professor) {
		professor->Update(deltaTime);
	}
	if (floor) {
		floor->Update(deltaTime);
	}
	if (ceiling) {
		ceiling->Update(deltaTime);
	}
}

void Floor1Scene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	if (!renderer) return;

	// 바닥 렌더링
	if (floor && floor->IsActive()) {
		glm::mat4 floorMatrix = floor->GetModelMat();
		renderer->RenderFBXModel("PlaneModel", floorMatrix, floor->GetColor());
	}

	// 천장 렌더링
	if (ceiling && ceiling->IsActive()) {
		glm::mat4 ceilingMatrix = ceiling->GetModelMat();
		renderer->RenderFBXModel("PlaneModel", ceilingMatrix, ceiling->GetColor());
	}

	// Professor 렌더링 (1층: RunSong - 애니메이션 + 텍스처)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();

		extern Engine* g_engine;
		FBXAnimationPlayer* animPlayer = g_engine ? g_engine->GetAnimationPlayer() : nullptr;

		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXModelWithAnimationAndTexture("RunSong", "RunSong", professorMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBXModelWithTexture("RunSong", "RunSong", professorMatrix);
		}
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
	std::cout << "Floor2Scene: Entered" << std::endl;

	// Professor 생성 및 초기화 (2층: RunLee)
	professor = std::make_unique<Professor>("RunLee", "RunAnimation", 0.6f, 1.8f, 0.6f);
	professor->SetPosition(glm::vec3(5.0f, 0.0f, 0.0f));

	// AnimationPlayer를 RunLee 모델로 초기화
	extern Engine* g_engine;
	if (g_engine) {
		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		ResourceManager* resMgr = g_engine->GetResourceManager();
		if (animPlayer && resMgr) {
			const FBXModel* model = resMgr->GetFBXModel("RunLee");
			if (model) {
				animPlayer->Init(model);
				if (!model->animations.empty()) {
					animPlayer->PlayAnimation(0);
					std::cout << "Floor2Scene: RunLee animation initialized" << std::endl;
				}
			}
		}
	}

	// 바닥 생성 및 초기화
	floor = std::make_unique<Plane>();
	floor->SetOrientation(Plane::Orientation::UP);
	floor->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f)); // 바닥을 y=-1 위치에 배치
	floor->SetSize(50.0f, 50.0f); // 50x50 크기의 바닥
	floor->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	floor->SetColor(glm::vec3(0.9f, 0.9f, 0.9f)); // 밝은 회색

	// 천장 생성 및 초기화
	ceiling = std::make_unique<Plane>();
	ceiling->SetOrientation(Plane::Orientation::DOWN);
	ceiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f)); // 천장을 y=5 위치에 배치
	ceiling->SetSize(50.0f, 50.0f); // 50x50 크기의 천장
	ceiling->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	ceiling->SetColor(glm::vec3(0.9f, 0.9f, 0.9f)); // 밝은 회색

	std::cout << "Floor2Scene: Professor (RunLee), Floor and Ceiling initialized" << std::endl;
}

void Floor2Scene::Exit()
{
	std::cout << "Floor2Scene: Exited" << std::endl;
	professor.reset();
	floor.reset();
	ceiling.reset();
}

void Floor2Scene::Update(float deltaTime)
{
	if (professor) {
		professor->Update(deltaTime);
	}
	if (floor) {
		floor->Update(deltaTime);
	}
	if (ceiling) {
		ceiling->Update(deltaTime);
	}
}

void Floor2Scene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	if (!renderer) return;

	// 바닥 렌더링
	if (floor && floor->IsActive()) {
		glm::mat4 floorMatrix = floor->GetModelMat();
		renderer->RenderFBXModel("PlaneModel", floorMatrix, floor->GetColor());
	}

	// 천장 렌더링
	if (ceiling && ceiling->IsActive()) {
		glm::mat4 ceilingMatrix = ceiling->GetModelMat();
		renderer->RenderFBXModel("PlaneModel", ceilingMatrix, ceiling->GetColor());
	}

	// Professor 렌더링 (2층: RunLee - 애니메이션 + 텍스처)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();

		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXModelWithAnimationAndTexture("RunLee", "RunLee", professorMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBXModelWithTexture("RunLee", "RunLee", professorMatrix);
		}
	}
}

//---------------------------------------------------------------Floor3Scene

void Floor3Scene::Enter()
{
	std::cout << "Floor3Scene: Entered" << std::endl;

	// Professor 생성 및 초기화 (3층: RunDragon)
	professor = std::make_unique<Professor>("RunDragon", "RunAnimation", 0.6f, 1.8f, 0.6f);
	professor->SetPosition(glm::vec3(5.0f, 0.0f, 0.0f));

	// AnimationPlayer를 RunDragon 모델로 초기화
	extern Engine* g_engine;
	if (g_engine) {
		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		ResourceManager* resMgr = g_engine->GetResourceManager();
		if (animPlayer && resMgr) {
			const FBXModel* model = resMgr->GetFBXModel("RunDragon");
			if (model) {
				animPlayer->Init(model);
				if (!model->animations.empty()) {
					animPlayer->PlayAnimation(0);
					std::cout << "Floor3Scene: RunDragon animation initialized" << std::endl;
				}
			}
		}
	}

	// 바닥 생성 및 초기화
	floor = std::make_unique<Plane>();
	floor->SetOrientation(Plane::Orientation::UP);
	floor->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f)); // 바닥을 y=-1 위치에 배치
	floor->SetSize(50.0f, 50.0f); // 50x50 크기의 바닥
	floor->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	floor->SetColor(glm::vec3(0.9f, 0.9f, 0.9f)); // 밝은 회색

	// 천장 생성 및 초기화
	ceiling = std::make_unique<Plane>();
	ceiling->SetOrientation(Plane::Orientation::DOWN);
	ceiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f)); // 천장을 y=5 위치에 배치
	ceiling->SetSize(50.0f, 50.0f); // 50x50 크기의 천장
	ceiling->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	ceiling->SetColor(glm::vec3(0.9f, 0.9f, 0.9f)); // 밝은 회색

	std::cout << "Floor3Scene: Professor (RunDragon), Floor and Ceiling initialized" << std::endl;
}

void Floor3Scene::Exit()
{
	std::cout << "Floor3Scene: Exited" << std::endl;
	professor.reset();
	floor.reset();
	ceiling.reset();
}

void Floor3Scene::Update(float deltaTime)
{
	if (professor) {
		professor->Update(deltaTime);
	}
	if (floor) {
		floor->Update(deltaTime);
	}
	if (ceiling) {
		ceiling->Update(deltaTime);
	}
}

void Floor3Scene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	if (!renderer) return;

	// 바닥 렌더링
	if (floor && floor->IsActive()) {
		glm::mat4 floorMatrix = floor->GetModelMat();
		renderer->RenderFBXModel("PlaneModel", floorMatrix, floor->GetColor());
	}

	// 천장 렌더링
	if (ceiling && ceiling->IsActive()) {
		glm::mat4 ceilingMatrix = ceiling->GetModelMat();
		renderer->RenderFBXModel("PlaneModel", ceilingMatrix, ceiling->GetColor());
	}

	// Professor 렌더링 (3층: RunDragon - 애니메이션 + 텍스처)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();

		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXModelWithAnimationAndTexture("RunDragon", "RunDragon", professorMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBXModelWithTexture("RunDragon", "RunDragon", professorMatrix);
		}
	}
}

//-----------------------------------------------------------------TestScene

void TestScene::Enter()
{
	std::cout << "TestScene: Entered" << std::endl;

	// Professor 객체 생성 및 초기화 (크기: 폭, 높이, 깊이)
	lee = std::make_unique<Professor>("RunLee", "RunAnimation", 1.0f, 2.0f, 1.0f);
	lee->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	lee->SetResourceID("RunLee");

	// Light 객체 생성 및 초기화
	light = std::make_unique<Light>(LightType::POINT);
	light->SetPosition(glm::vec3(5.0f, 5.0f, 5.0f));
	light->SetDiffuse(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetAmbient(glm::vec3(0.3f, 0.3f, 0.3f));
	light->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetEnabled(true);

	std::cout << "TestScene: Professor 'lee' and Light initialized" << std::endl;
}

void TestScene::Exit()
{
	std::cout << "TestScene: Exited" << std::endl;
	lee.reset();
	light.reset();
}

void TestScene::Update(float deltaTime)
{
	if (lee) {
		lee->Update(deltaTime);
	}
}

void TestScene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();

	if (!renderer) return;

	// Professor 'lee' 렌더링
	if (lee && lee->IsActive()) {
		glm::mat4 modelMatrix = lee->GetModelMat();

		// 애니메이션 + 텍스처와 함께 렌더링
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXModelWithAnimationAndTexture("RunLee", "RunLee", modelMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBXModelWithTexture("RunLee", "RunLee", modelMatrix);
		}
	}
}
