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
#include "Wall.h"
#include "GameConstants.h"
#include "MapGenerator.h"
#include "CollisionManager.h"
#include "AiController.h"
#include "NavMeshBuilder.h"
#include "NavMesh.h"
#include "PathFinder.h"

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
	std::cout << "\n========== SCENE CHANGE REQUEST ==========" << std::endl;
	std::cout << "Requested scene: " << sceneName << std::endl;

	auto it = sceneFactory.find(sceneName);
	if (it not_eq sceneFactory.end()) {
		// 이전 씬 종료
		if (currentScene) {
			std::cout << "Exiting current scene..." << std::endl;
			currentScene->Exit();
			std::cout << "Current scene exited successfully" << std::endl;
		}

		// 새로운 씬 생성 및 진입
		std::cout << "Creating new scene: " << sceneName << std::endl;
		currentScene = it->second();
		std::cout << "New scene created, calling Enter()..." << std::endl;
		currentScene->Enter();
		std::cout << "SceneManager: Successfully changed to " << sceneName << " scene" << std::endl;
	}
	else {
		std::cerr << "SceneManager: ERROR - Scene '" << sceneName << "' not found!" << std::endl;
	}
	std::cout << "=========================================\n" << std::endl;
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

	// OpenGL 상태 초기화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);

	// Camera 위치 복원 (다른 씬에서 변경되었을 수 있음)
	extern Engine* g_engine;
	if (g_engine) {
		Camera* camera = g_engine->GetCamera();
		if (camera) {
			// Camera를 Engine 초기화 시와 동일한 상태로 복원
			camera->SetPosition(glm::vec3(0.0f, 2.0f, 5.0f));
			camera->SetDirection(glm::vec3(0.0f, 0.0f, 0.0f));
			std::cout << "TitleScene: Camera reset to (0, 2, 5) looking at (0, 0, 0)" << std::endl;
		}
	}

	// Light 생성 (렌더링을 위해 필수)
	light = std::make_unique<Light>(LightType::POINT);
	light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	light->SetDiffuse(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetAmbient(glm::vec3(0.8f, 0.8f, 0.8f));  // 밝은 Ambient로 설정
	light->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetEnabled(true);

	// 타이틀용 Plane 생성 (카메라 앞에 배치)
	titlePlane = std::make_unique<Plane>();
	titlePlane->SetOrientation(Plane::Orientation::FRONT);  // 카메라를 향하도록
	titlePlane->SetPosition(glm::vec3(0.0f, 2.0f, -3.0f));  // 카메라 앞 3미터
	titlePlane->SetSize(3.0f, 2.0f);  // 3x2 크기의 평면 (더 크게)
	titlePlane->SetResourceID("PlaneModel");
	titlePlane->SetColor(glm::vec3(1.0f, 1.0f, 1.0f));  // 흰색

	std::cout << "TitleScene: Plane created at (0, 2, -3) with size 3x2" << std::endl;

	// InputManager 액션 바인딩 (멤버 변수 사용)
	if (g_engine) {
		InputManager* inputMgr = g_engine->GetInputManager();
		if (inputMgr) {
			inputMgr->ActionW = [this]() { keyPressed = true; };
			inputMgr->ActionA = [this]() { keyPressed = true; };
			inputMgr->ActionS = [this]() { keyPressed = true; };
			inputMgr->ActionD = [this]() { keyPressed = true; };
		}
	}

	std::cout << "TitleScene: Title plane and light initialized" << std::endl;
}

void TitleScene::Exit()
{
	std::cout << "TitleScene: Exited" << std::endl;

	// OpenGL 3.3 상태 리셋
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	// glClearColor는 Renderer에서 관리하므로 여기서 변경하지 않음

	std::cout << "TitleScene: OpenGL 3.3 state reset" << std::endl;

	// 객체 정리
	titlePlane.reset();
	light.reset();

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
	// OpenGL 3.3 코어 프로파일에서는 레거시 텍스트 렌더링 불가
	// 임시로 색깔있는 화면으로 렌더링이 되는지 확인

	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	if (!renderer) return;

	// Light를 Renderer에 설정
	if (light) {
		renderer->SetLight(light.get());
	}

	// 간단한 Plane을 렌더링해서 뭔가 보이는지 확인
	if (titlePlane && titlePlane->IsActive()) {
		glm::mat4 planeMatrix = titlePlane->GetModelMat();
		// 깜빡이는 흰색 Plane 렌더링
		glm::vec3 fadeColor(alpha, alpha, alpha);
		renderer->RenderObj("PlaneModel", planeMatrix, fadeColor);
	}

	// 콘솔에 메시지 출력
	static bool messagePrinted = false;
	if (!messagePrinted) {
		std::cout << "\n========================================" << std::endl;
		std::cout << "    PRESS ANY KEY TO START" << std::endl;
		std::cout << "    (White flashing plane should be visible)" << std::endl;
		std::cout << "========================================\n" << std::endl;
		messagePrinted = true;
	}
}

//---------------------------------------------------------------Floor1Scene

void Floor1Scene::Enter()
{
	std::cout << "Floor1Scene: Entered" << std::endl;

	// OpenGL 상태 확실히 초기화 (현대 OpenGL용)
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	std::cout << "Floor1Scene: OpenGL state initialized" << std::endl;

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
	player->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	player->SetResourceID("PlayerModel");

	// Camera 설정 - 플레이어 눈 높이에서 앞쪽(-Z)을 약간 아래로 바라봄
	if (camera) {
		camera->SetPosition(glm::vec3(0.0f, 1.5f, 0.0f));  // 플레이어 눈 높이
		camera->SetDirection(glm::vec3(0.0f, 0.5f, -5.0f)); // 앞쪽 아래를 바라봄 (바닥 보임)
		std::cout << "Floor1Scene: Camera set to look forward and down" << std::endl;
	}

	// 이제 카메라를 플레이어에 연결
	player->Init(camera);

	// Professor 생성 및 초기화 (1층: RunSong) - 카메라 앞쪽에 배치
	professor = std::make_unique<Professor>("RunSong", "RunAnimation", 0.6f, 1.8f, 0.6f);
	professor->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));  // 카메라 앞쪽 5미터
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
	floor->SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT); // 바닥 크기
	floor->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	floor->SetTextureID("FloorTexture"); // 바닥 텍스처 설정
	floor->SetTextureTiling(glm::vec2(GameConstants::FLOOR_TEXTURE_TILE_X, GameConstants::FLOOR_TEXTURE_TILE_Y)); // 타일링 설정
	floor->SetColor(glm::vec3(0.6f, 0.5f, 0.4f)); // 밝은 갈색 (바닥)

	// 천장 생성 및 초기화
	ceiling = std::make_unique<Plane>();
	ceiling->SetOrientation(Plane::Orientation::DOWN);
	ceiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f)); // 천장을 y=5 위치에 배치
	ceiling->SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT); // 천장 크기
	ceiling->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	ceiling->SetTextureID("CeilingTexture"); // 천장 텍스처 설정
	ceiling->SetTextureTiling(glm::vec2(GameConstants::CEILING_TEXTURE_TILE_X, GameConstants::CEILING_TEXTURE_TILE_Y)); // 타일링 설정
	ceiling->SetColor(glm::vec3(0.8f, 0.8f, 0.8f)); // 밝은 회색 (천장)

	// 테스트용 벽 생성 - 카메라 앞쪽 왼편에 배치
	testWall = std::make_unique<Wall>();
	testWall->SetGridPosition(3, 3); // 그리드 (3, 3) 위치에 배치 (카메라 앞쪽)
	// Wall 생성자에서 이미 CubeModel, WallTexture 설정됨

	// InputManager 액션을 Player 이동에 연결
	if (inputMgr && timer) {
		inputMgr->ActionW = [this, timer]() { if (player) player->MoveForward(timer->elapsedTime); };
		inputMgr->ActionS = [this, timer]() { if (player) player->MoveBackward(timer->elapsedTime); };
		inputMgr->ActionA = [this, timer]() { if (player) player->MoveLeft(timer->elapsedTime); };
		inputMgr->ActionD = [this, timer]() { if (player) player->MoveRight(timer->elapsedTime); };
	}

	// Light 객체 생성 및 초기화
	light = std::make_unique<Light>(LightType::POINT);
	light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	light->SetDiffuse(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetAmbient(glm::vec3(0.5f, 0.5f, 0.5f));
	light->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetEnabled(true);

	std::cout << "Floor1Scene: Player and Professor initialized" << std::endl;
}

void Floor1Scene::Exit()
{
	std::cout << "Floor1Scene: Exited" << std::endl;

	// CollisionManager 정리
	extern Engine* g_engine;
	if (g_engine) {
		CollisionManager* collisionMgr = g_engine->GetCollisionManager();
		if (collisionMgr) {
			collisionMgr->ClearAll();
			std::cout << "Floor1Scene: CollisionManager cleared" << std::endl;
		}

		// InputManager 액션 해제
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
	testWall.reset();
	light.reset();
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
	if (testWall) {
		testWall->Update(deltaTime);
	}
}

void Floor1Scene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	Camera* camera = g_engine->GetCamera();
	if (!renderer) return;

	// Light를 Renderer에 설정
	if (light) {
		renderer->SetLight(light.get());
	}

	// 카메라 디버그 출력 (한 번만)
	static bool cameraDebugPrinted = false;
	if (!cameraDebugPrinted && camera) {
		std::cout << "\n===== FLOOR1 CAMERA DEBUG =====" << std::endl;
		glm::vec3 camPos = camera->GetPosition();
		glm::vec3 camDir = camera->GetDirection();
		std::cout << "Camera Position: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")" << std::endl;
		std::cout << "Camera Direction (target): (" << camDir.x << ", " << camDir.y << ", " << camDir.z << ")" << std::endl;
		if (light) {
			std::cout << "Light Position: (" << light->GetPosition().x << ", " << light->GetPosition().y << ", " << light->GetPosition().z << ")" << std::endl;
		}

		// OpenGL 상태 체크
		GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		std::cout << "GL_DEPTH_TEST: " << (depthTest ? "ENABLED" : "DISABLED") << std::endl;
		std::cout << "Viewport: " << viewport[0] << ", " << viewport[1] << ", " << viewport[2] << ", " << viewport[3] << std::endl;

		std::cout << "===============================\n" << std::endl;
		cameraDebugPrinted = true;
	}

	// 바닥 렌더링 (백페이스 컬링 비활성화)
	static bool floorDebugPrinted = false;
	if (floor && floor->IsActive()) {
		if (!floorDebugPrinted) {
			std::cout << "Floor1Scene: Rendering floor..." << std::endl;
			std::cout << "  Position: (" << floor->GetPosition().x << ", " << floor->GetPosition().y << ", " << floor->GetPosition().z << ")" << std::endl;
			std::cout << "  Size: " << floor->GetSize().x << "x" << floor->GetSize().y << std::endl;
			std::cout << "  TextureID: " << (floor->GetTextureID().empty() ? "NONE" : floor->GetTextureID()) << std::endl;
			std::cout << "  IsActive: " << floor->IsActive() << std::endl;
			floorDebugPrinted = true;
		}
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 floorMatrix = floor->GetModelMat();
		if (!floor->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", floor->GetTextureID(), floorMatrix, floor->GetTextureTiling());
		} else {
			renderer->RenderObj("PlaneModel", floorMatrix, floor->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	} else {
		static bool floorMissingPrinted = false;
		if (!floorMissingPrinted) {
			std::cerr << "Floor1Scene: Floor is NULL or not active!" << std::endl;
			floorMissingPrinted = true;
		}
	}

	// 천장 렌더링 (백페이스 컬링 비활성화)
	static bool ceilingDebugPrinted = false;
	if (ceiling && ceiling->IsActive()) {
		if (!ceilingDebugPrinted) {
			std::cout << "Floor1Scene: Rendering ceiling..." << std::endl;
			std::cout << "  Position: (" << ceiling->GetPosition().x << ", " << ceiling->GetPosition().y << ", " << ceiling->GetPosition().z << ")" << std::endl;
			std::cout << "  Size: " << ceiling->GetSize().x << "x" << ceiling->GetSize().y << std::endl;
			std::cout << "  TextureID: " << (ceiling->GetTextureID().empty() ? "NONE" : ceiling->GetTextureID()) << std::endl;
			std::cout << "  IsActive: " << ceiling->IsActive() << std::endl;
			ceilingDebugPrinted = true;
		}
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 ceilingMatrix = ceiling->GetModelMat();
		if (!ceiling->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", ceiling->GetTextureID(), ceilingMatrix, ceiling->GetTextureTiling());
		} else {
			renderer->RenderObj("PlaneModel", ceilingMatrix, ceiling->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	} else {
		static bool ceilingMissingPrinted = false;
		if (!ceilingMissingPrinted) {
			std::cerr << "Floor1Scene: Ceiling is NULL or not active!" << std::endl;
			ceilingMissingPrinted = true;
		}
	}

	// 테스트 벽 렌더링
	if (testWall && testWall->IsActive()) {
		glm::mat4 wallMatrix = testWall->GetModelMat();
		// 텍스처가 설정되어 있으면 텍스처와 함께 렌더링
		if (!testWall->GetTextureID().empty()) {
			renderer->RenderObjWithTexture(testWall->GetResourceID(), testWall->GetTextureID(), wallMatrix);
		} else {
			renderer->RenderObj(testWall->GetResourceID(), wallMatrix, testWall->GetColor());
		}
	}

	// Professor 렌더링 (1층: RunSong - 애니메이션 + 텍스처)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();

		extern Engine* g_engine;
		FBXAnimationPlayer* animPlayer = g_engine ? g_engine->GetAnimationPlayer() : nullptr;

		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXAnimated("RunSong", "RunSong", professorMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBX("RunSong", "RunSong", professorMatrix);
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

	// OpenGL 상태 확실히 초기화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	std::cout << "Floor2Scene: OpenGL state initialized" << std::endl;

	extern Engine* g_engine;
	if (!g_engine) {
		std::cerr << "ERROR: Floor2Scene - g_engine is null" << std::endl;
		return;
	}

	Camera* camera = g_engine->GetCamera();

	// Camera 설정 - 앞쪽(-Z)을 약간 아래로 바라봄
	if (camera) {
		camera->SetPosition(glm::vec3(0.0f, 1.5f, 0.0f));
		camera->SetDirection(glm::vec3(0.0f, 0.5f, -5.0f));
		std::cout << "Floor2Scene: Camera set to look forward and down" << std::endl;
	}

	// Professor 생성 및 초기화 (2층: RunLee) - 카메라 앞쪽에 배치
	professor = std::make_unique<Professor>("RunLee", "RunAnimation", 0.6f, 1.8f, 0.6f);
	professor->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));  // 카메라 앞쪽 5미터

	// AnimationPlayer를 RunLee 모델로 초기화
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

	// 바닥 생성 및 초기화
	floor = std::make_unique<Plane>();
	floor->SetOrientation(Plane::Orientation::UP);
	floor->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f)); // 바닥을 y=-1 위치에 배치
	floor->SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT); // 바닥 크기
	floor->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	floor->SetTextureID("FloorTexture"); // 바닥 텍스처 설정
	floor->SetTextureTiling(glm::vec2(GameConstants::FLOOR_TEXTURE_TILE_X, GameConstants::FLOOR_TEXTURE_TILE_Y)); // 타일링 설정
	floor->SetColor(glm::vec3(0.6f, 0.5f, 0.4f)); // 밝은 갈색 (바닥)

	// 천장 생성 및 초기화
	ceiling = std::make_unique<Plane>();
	ceiling->SetOrientation(Plane::Orientation::DOWN);
	ceiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f)); // 천장을 y=5 위치에 배치
	ceiling->SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT); // 천장 크기
	ceiling->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	ceiling->SetTextureID("CeilingTexture"); // 천장 텍스처 설정
	ceiling->SetTextureTiling(glm::vec2(GameConstants::CEILING_TEXTURE_TILE_X, GameConstants::CEILING_TEXTURE_TILE_Y)); // 타일링 설정
	ceiling->SetColor(glm::vec3(0.7f, 0.7f, 0.7f)); // 밝은 회색 (천장)

	// Light 객체 생성 및 초기화
	light = std::make_unique<Light>(LightType::POINT);
	light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	light->SetDiffuse(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetAmbient(glm::vec3(0.5f, 0.5f, 0.5f));
	light->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetEnabled(true);

	std::cout << "Floor2Scene: Professor (RunLee), Floor and Ceiling initialized" << std::endl;
}

void Floor2Scene::Exit()
{
	std::cout << "Floor2Scene: Exited" << std::endl;

	// CollisionManager 정리
	extern Engine* g_engine;
	if (g_engine) {
		CollisionManager* collisionMgr = g_engine->GetCollisionManager();
		if (collisionMgr) {
			collisionMgr->ClearAll();
			std::cout << "Floor2Scene: CollisionManager cleared" << std::endl;
		}
	}

	professor.reset();
	floor.reset();
	ceiling.reset();
	light.reset();
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

	// Light를 Renderer에 설정
	if (light) {
		renderer->SetLight(light.get());
	}

	// 바닥 렌더링 (백페이스 컬링 비활성화)
	if (floor && floor->IsActive()) {
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 floorMatrix = floor->GetModelMat();
		// 텍스처가 설정되어 있으면 텍스처와 함께 렌더링, 아니면 컬러로 렌더링
		if (!floor->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", floor->GetTextureID(), floorMatrix, floor->GetTextureTiling());
		} else {
			renderer->RenderObj("PlaneModel", floorMatrix, floor->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	}

	// 천장 렌더링 (백페이스 컬링 비활성화)
	if (ceiling && ceiling->IsActive()) {
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 ceilingMatrix = ceiling->GetModelMat();
		// 텍스처가 설정되어 있으면 텍스처와 함께 렌더링, 아니면 컬러로 렌더링
		if (!ceiling->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", ceiling->GetTextureID(), ceilingMatrix, ceiling->GetTextureTiling());
		} else {
			renderer->RenderObj("PlaneModel", ceilingMatrix, ceiling->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	}

	// Professor 렌더링 (2층: RunLee - 애니메이션 + 텍스처)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();

		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXAnimated("RunLee", "RunLee", professorMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBX("RunLee", "RunLee", professorMatrix);
		}
	}
}

//---------------------------------------------------------------Floor3Scene

void Floor3Scene::Enter()
{
	std::cout << "Floor3Scene: Entered" << std::endl;

	// OpenGL 상태 확실히 초기화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	std::cout << "Floor3Scene: OpenGL state initialized" << std::endl;

	extern Engine* g_engine;
	if (!g_engine) {
		std::cerr << "ERROR: Floor3Scene - g_engine is null" << std::endl;
		return;
	}

	Camera* camera = g_engine->GetCamera();

	// Camera 설정 - 앞쪽(-Z)을 약간 아래로 바라봄
	if (camera) {
		camera->SetPosition(glm::vec3(0.0f, 1.5f, 0.0f));
		camera->SetDirection(glm::vec3(0.0f, 0.5f, -5.0f));
		std::cout << "Floor3Scene: Camera set to look forward and down" << std::endl;
	}

	// Professor 생성 및 초기화 (3층: RunDragon) - 카메라 앞쪽에 배치
	professor = std::make_unique<Professor>("RunDragon", "RunAnimation", 0.6f, 1.8f, 0.6f);
	professor->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));  // 카메라 앞쪽 5미터

	// AnimationPlayer를 RunDragon 모델로 초기화
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

	// 바닥 생성 및 초기화
	floor = std::make_unique<Plane>();
	floor->SetOrientation(Plane::Orientation::UP);
	floor->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f)); // 바닥을 y=-1 위치에 배치
	floor->SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT); // 바닥 크기
	floor->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	floor->SetTextureID("FloorTexture"); // 바닥 텍스처 설정
	floor->SetTextureTiling(glm::vec2(GameConstants::FLOOR_TEXTURE_TILE_X, GameConstants::FLOOR_TEXTURE_TILE_Y)); // 타일링 설정
	floor->SetColor(glm::vec3(0.6f, 0.5f, 0.4f)); // 밝은 갈색 (바닥)

	// 천장 생성 및 초기화
	ceiling = std::make_unique<Plane>();
	ceiling->SetOrientation(Plane::Orientation::DOWN);
	ceiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f)); // 천장을 y=5 위치에 배치
	ceiling->SetSize(GameConstants::FLOOR_DEFAULT_WIDTH, GameConstants::FLOOR_DEFAULT_HEIGHT); // 천장 크기
	ceiling->SetResourceID("PlaneModel"); // Plane 메쉬 리소스 ID
	ceiling->SetTextureID("CeilingTexture"); // 천장 텍스처 설정
	ceiling->SetTextureTiling(glm::vec2(GameConstants::CEILING_TEXTURE_TILE_X, GameConstants::CEILING_TEXTURE_TILE_Y)); // 타일링 설정
	ceiling->SetColor(glm::vec3(0.7f, 0.7f, 0.7f)); // 밝은 회색 (천장)

	// Light 객체 생성 및 초기화
	light = std::make_unique<Light>(LightType::POINT);
	light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	light->SetDiffuse(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetAmbient(glm::vec3(0.5f, 0.5f, 0.5f));
	light->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetEnabled(true);

	std::cout << "Floor3Scene: Professor (RunDragon), Floor and Ceiling initialized" << std::endl;
}

void Floor3Scene::Exit()
{
	std::cout << "Floor3Scene: Exited" << std::endl;

	// CollisionManager 정리
	extern Engine* g_engine;
	if (g_engine) {
		CollisionManager* collisionMgr = g_engine->GetCollisionManager();
		if (collisionMgr) {
			collisionMgr->ClearAll();
			std::cout << "Floor3Scene: CollisionManager cleared" << std::endl;
		}
	}

	professor.reset();
	floor.reset();
	ceiling.reset();
	light.reset();
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

	// Light를 Renderer에 설정
	if (light) {
		renderer->SetLight(light.get());
	}

	// 바닥 렌더링 (백페이스 컬링 비활성화)
	if (floor && floor->IsActive()) {
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 floorMatrix = floor->GetModelMat();
		// 텍스처가 설정되어 있으면 텍스처와 함께 렌더링, 아니면 컬러로 렌더링
		if (!floor->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", floor->GetTextureID(), floorMatrix, floor->GetTextureTiling());
		} else {
			renderer->RenderObj("PlaneModel", floorMatrix, floor->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	}

	// 천장 렌더링 (백페이스 컬링 비활성화)
	if (ceiling && ceiling->IsActive()) {
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 ceilingMatrix = ceiling->GetModelMat();
		// 텍스처가 설정되어 있으면 텍스처와 함께 렌더링, 아니면 컬러로 렌더링
		if (!ceiling->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", ceiling->GetTextureID(), ceilingMatrix, ceiling->GetTextureTiling());
		} else {
			renderer->RenderObj("PlaneModel", ceilingMatrix, ceiling->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	}

	// Professor 렌더링 (3층: RunDragon - 애니메이션 + 텍스처)
	if (professor && professor->IsActive()) {
		glm::mat4 professorMatrix = professor->GetModelMat();

		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXAnimated("RunDragon", "RunDragon", professorMatrix, animPlayer->GetBoneTransforms());
		} else {
			renderer->RenderFBX("RunDragon", "RunDragon", professorMatrix);
		}
	}
}

//-----------------------------------------------------------------TestScene

void TestScene::Enter()
{
	std::cout << "TestScene: Entered" << std::endl;

	// OpenGL 상태 초기화
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
	std::cout << "TestScene: OpenGL state initialized" << std::endl;

	// ============================================
	// 1단계: 맵 생성 (매번 다른 미로)
	// ============================================
	mapGenerator = std::make_unique<MapGenerator>(GameConstants::MAP_GRID_WIDTH, GameConstants::MAP_GRID_DEPTH);
	mapGenerator->Generate();
	mapGenerator->PrintMap();

	std::cout << "\n===== MAP GENERATION COMPLETE =====" << std::endl;
	std::cout << "New random maze generated for this floor" << std::endl;
	std::cout << "=====================================\n" << std::endl;

	// 플레이어 시작 위치 찾기
	glm::vec3 playerStartPos(0.0f, 0.0f, 0.0f);
	bool foundStartPos = false;
	for (int z = 0; z < GameConstants::MAP_GRID_DEPTH && !foundStartPos; ++z) {
		for (int x = 0; x < GameConstants::MAP_GRID_WIDTH && !foundStartPos; ++x) {
			TileType tile = mapGenerator->GetTile(x, z);
			if (tile == TileType::STAIR) {
				float halfMapSize = (GameConstants::MAP_GRID_WIDTH * GameConstants::TILE_SIZE) * 0.5f;
				float worldX = (x * GameConstants::TILE_SIZE) - halfMapSize + (GameConstants::TILE_SIZE * 0.5f);
				float worldZ = (z * GameConstants::TILE_SIZE) - halfMapSize + (GameConstants::TILE_SIZE * 0.5f);
				playerStartPos = glm::vec3(worldX, 0.0f, worldZ);
				foundStartPos = true;

				std::cout << "Player spawn at grid [" << x << "," << z << "]" << std::endl;
				std::cout << "World position: (" << worldX << ", 0, " << worldZ << ")" << std::endl;
			}
		}
	}

	// ============================================
	// 2단계: Wall 객체 생성 (실제 3D 벽)
	// ============================================
	walls.clear();
	for (int z = 0; z < GameConstants::MAP_GRID_DEPTH; ++z) {
		for (int x = 0; x < GameConstants::MAP_GRID_WIDTH; ++x) {
			TileType tile = mapGenerator->GetTile(x, z);
			if (tile == TileType::WALL) {
				auto wall = std::make_unique<Wall>();
				wall->SetGridPosition(x, z);
				walls.push_back(std::move(wall));
			}
		}
	}

	std::cout << "\n===== WALL PLACEMENT =====" << std::endl;
	std::cout << "Total walls placed: " << walls.size() << std::endl;
	std::cout << "==========================\n" << std::endl;

	// Player 생성
	extern Engine* g_engine;
	if (g_engine) {
		Camera* camera = g_engine->GetCamera();
		InputManager* inputMgr = g_engine->GetInputManager();
		GameTimer* timer = g_engine->GetGameTimer();

		glm::vec3 initialCameraPos = playerStartPos;
		initialCameraPos.y = GameConstants::PLAYER_EYE_HEIGHT;

		if (camera) {
			camera->SetPosition(initialCameraPos);
			camera->SetDirection(initialCameraPos + glm::vec3(0.0f, 0.0f, -5.0f));
			camera->SetMoveSpeed(50.0f);
		}

		player = std::make_unique<Player>();
		player->Init(camera);
		player->SetPosition(playerStartPos);
		player->SetResourceID("PlayerModel");
		player->SetMoveSpeed(50.0f);

		// InputManager 액션 설정
		if (inputMgr && timer) {
			inputMgr->ActionW = [this, timer]() { if (player) player->MoveForward(timer->elapsedTime); };
			inputMgr->ActionS = [this, timer]() { if (player) player->MoveBackward(timer->elapsedTime); };
			inputMgr->ActionA = [this, timer]() { if (player) player->MoveLeft(timer->elapsedTime); };
			inputMgr->ActionD = [this, timer]() { if (player) player->MoveRight(timer->elapsedTime); };
			inputMgr->ActionSpace = [this, timer]() {
				if (player) {
					glm::vec3 pos = player->GetPosition();
					pos.y += 50.0f * timer->elapsedTime;
					player->SetPosition(pos);
				}
				};
			inputMgr->ActionShift = [this, timer]() {
				if (player) {
					glm::vec3 pos = player->GetPosition();
					pos.y -= 50.0f * timer->elapsedTime;
					player->SetPosition(pos);
				}
				};
		}

		// ============================================
		// 3단계: NavMesh 동적 생성 ⭐⭐⭐
		// ============================================
		std::cout << "\n===== NAVMESH BUILDING =====" << std::endl;
		std::cout << "Building NavMesh based on actual 3D walls..." << std::endl;

		// NavMeshBuilder로 NavMesh 생성
		NavMeshBuilder navMeshBuilder;
		navMeshBuilder.BuildFromWalls(&walls, playerStartPos, GameConstants::TILE_SIZE);

		// ⭐⭐⭐ 소유권 이전! (NavMeshBuilder가 소멸되어도 NavMesh는 유지됨)
		navMesh = navMeshBuilder.ReleaseMesh();

		// ⚠️ 주의: GetNavMesh()가 포인터를 반환하므로, 
		// NavMeshBuilder가 소멸될 때 NavMesh도 같이 소멸됨!
		// 해결책: NavMeshBuilder에서 소유권을 이전하도록 수정 필요

		std::cout << "NavMesh created with " << (navMesh ? navMesh->GetAllNodes().size() : 0) << " walkable nodes" << std::endl;
		std::cout << "==============================\n" << std::endl;

		// ============================================
		// 4단계: Professor (NPC) 생성 + AI 초기화
		// ============================================
		std::cout << "\n===== PROFESSOR INITIALIZATION =====" << std::endl;

		lee = std::make_unique<Professor>("RunLee", "RunAnimation", 0.6f, 1.8f, 0.6f);

		// ⭐ Professor를 NavMesh에서 이동 가능한 위치에 배치
		glm::vec3 professorPos = playerStartPos; // 일단 플레이어 위치에서 시작

		// NavMesh에서 이동 가능한 근처 타일 찾기
		if (navMesh) {  // ⭐ tempNavMesh → navMesh로 변경!
			bool foundWalkableTile = false;

			// ⭐ 플레이어 바로 옆 (반경 2 타일 이내)에서 이동 가능한 타일 찾기
			for (int offsetZ = -2; offsetZ <= 2 && !foundWalkableTile; ++offsetZ) {
				for (int offsetX = -2; offsetX <= 2 && !foundWalkableTile; ++offsetX) {
					// 플레이어와 최소 1타일은 떨어지도록 (너무 가까우면 겹침)
					if (std::abs(offsetX) < 1 && std::abs(offsetZ) < 1) {
						continue;
					}

					// ⭐ 월드 좌표 계산 (TILE_SIZE 사용!)
					float testX = playerStartPos.x + (offsetX * GameConstants::TILE_SIZE);
					float testZ = playerStartPos.z + (offsetZ * GameConstants::TILE_SIZE);
					glm::vec3 testPos(testX, 0.0f, testZ);

					// NavMesh에서 이 위치의 노드 확인
					NavNode* testNode = navMesh->GetNodeFromWorldPos(testPos);
					if (testNode && testNode->IsWalkable()) {
						// 이동 가능한 타일 발견!
						professorPos = testNode->GetWorldPosition();
						foundWalkableTile = true;
						std::cout << "[V] Found walkable tile for Professor at: ("
							<< professorPos.x << ", " << professorPos.y << ", " << professorPos.z << ")" << std::endl;
					}
				}
			}

			if (!foundWalkableTile) {
				std::cerr << "[!!] WARNING: Could not find walkable tile for Professor near player!" << std::endl;
				// ⭐ 플레이어 위치 그대로 사용 (최후의 수단)
				professorPos = playerStartPos;
			}
		}

		lee->SetPosition(professorPos);
		lee->SetPlayerReference(player.get());

		// FBXAnimationPlayer 초기화
		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		ResourceManager* resMgr = g_engine->GetResourceManager();
		if (animPlayer && resMgr) {
			const FBXModel* model = resMgr->GetFBXModel("RunLee");
			if (model) {
				animPlayer->Init(model);
				if (!model->animations.empty()) {
					animPlayer->PlayAnimation(0);
				}
			}
		}

		// ⭐ AIController 초기화 - NavMesh 연동
		if (navMesh && !navMesh->GetAllNodes().empty()) {  // ⭐ tempNavMesh → navMesh
			// PathFinder 생성 (NavMesh 포인터 전달)
			PathFinder* pathFinder = new PathFinder(navMesh.get());  // ⭐ .get() 사용
			lee->SetPathFinder(pathFinder);

			// AIController 생성
			AIController* aiController = new AIController(pathFinder);
			aiController->SetCurrentPosition(professorPos);
			lee->SetAIController(aiController);

			// 도망칠 목표 지점 설정
			glm::vec3 escapeTarget = professorPos;

			// ⭐ Professor 주변 가까운 곳에서 목표 지점 찾기 (5~8 타일 거리)
			bool foundEscapeTarget = false;
			for (int offsetZ = -8; offsetZ <= 8 && !foundEscapeTarget; ++offsetZ) {
				for (int offsetX = -8; offsetX <= 8 && !foundEscapeTarget; ++offsetX) {
					// 거리 확인 (최소 5 타일 이상 떨어진 곳)
					if (std::abs(offsetX) < 5 || std::abs(offsetZ) < 5) {
						continue;
					}

					// ⭐ 월드 좌표 계산 (TILE_SIZE 사용!)
					float testX = professorPos.x + (offsetX * GameConstants::TILE_SIZE);
					float testZ = professorPos.z + (offsetZ * GameConstants::TILE_SIZE);
					glm::vec3 testPos(testX, 0.0f, testZ);

					NavNode* testNode = navMesh->GetNodeFromWorldPos(testPos);
					if (testNode && testNode->IsWalkable()) {
						escapeTarget = testNode->GetWorldPosition();
						foundEscapeTarget = true;
						std::cout << "[V] Found escape target at: ("
							<< escapeTarget.x << ", " << escapeTarget.y << ", " << escapeTarget.z << ")" << std::endl;
					}
				}
			}

			if (!foundEscapeTarget) {
				std::cerr << "[!!] WARNING: Could not find escape target! Using random direction." << std::endl;
				// 최후의 수단: Professor에서 임의 방향으로 20m
				escapeTarget = professorPos + glm::vec3(20.0f, 0.0f, 20.0f);
			}

			lee->SetPatrolTarget(escapeTarget);

			std::cout << "[V] AIController initialized" << std::endl;
			std::cout << "[V] PathFinder initialized with NavMesh" << std::endl;
			std::cout << "[V] NavMesh: " << navMesh->GetAllNodes().size() << " nodes" << std::endl;
			std::cout << "[V] Professor position: (" << professorPos.x << ", " << professorPos.y << ", " << professorPos.z << ")" << std::endl;
			std::cout << "[V] AIController position: (" << aiController->GetCurrentPosition().x << ", " << aiController->GetCurrentPosition().y << ", " << aiController->GetCurrentPosition().z << ")" << std::endl;
			std::cout << "[V] Patrol target set to: (" << escapeTarget.x << ", " << escapeTarget.y << ", " << escapeTarget.z << ")" << std::endl;
		}
		else {
			std::cerr << "[X] WARNING: NavMesh is empty! AI pathfinding disabled" << std::endl;
		}

		std::cout << "===================================\n" << std::endl;

		// CollisionManager 설정
		CollisionManager* collisionMgr = g_engine->GetCollisionManager();
		if (collisionMgr) {
			collisionMgr->RegisterStaticObjects(&walls);
			collisionMgr->RegisterDynamicObject(player.get());
			std::cout << "Collision detection enabled\n" << std::endl;
		}
	}

	// 바닥 생성
	floor = std::make_unique<Plane>();
	floor->SetOrientation(Plane::Orientation::UP);
	floor->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	float mapSize = GameConstants::MAP_GRID_WIDTH * GameConstants::TILE_SIZE;
	floor->SetSize(mapSize, mapSize);
	floor->SetResourceID("PlaneModel");
	floor->SetTextureID("FloorTexture");
	floor->SetTextureTiling(glm::vec2(GameConstants::FLOOR_TEXTURE_TILE_X, GameConstants::FLOOR_TEXTURE_TILE_Y));

	// 레거시 단일 조명 (하위 호환성)
	light = std::make_unique<Light>(LightType::POINT);
	light->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
	light->SetDiffuse(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetAmbient(glm::vec3(0.5f, 0.5f, 0.5f));
	light->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));
	light->SetEnabled(true);

	// 다중 조명 배치 (최대 8개까지 가능)

	// 1. 방향성 조명 (Directional Light) - 태양광 같은 전역 조명
	auto dirLight = std::make_unique<Light>(LightType::DIRECTIONAL);
	dirLight->SetDirection(glm::vec3(-0.2f, -1.0f, -0.3f));  // 방향: 왼쪽 위에서 아래로 비추는 각도
	dirLight->SetAmbient(glm::vec3(0.2f, 0.2f, 0.25f));      // 주변광: 약한 파란빛 (밤하늘 분위기)
	dirLight->SetDiffuse(glm::vec3(0.5f, 0.5f, 0.6f));       // 확산광: 중간 강도의 차가운 흰색
	dirLight->SetSpecular(glm::vec3(0.3f, 0.3f, 0.3f));      // 반사광: 약한 하이라이트
	dirLight->SetIntensity(0.8f);                             // 강도: 80% (0.0 ~ 1.0)
	dirLight->SetEnabled(true);                               // 활성화
	lights.push_back(std::move(dirLight));

	// 2. 포인트 조명 1 (Point Light) - 맵 중앙 위쪽의 메인 조명
	auto pointLight1 = std::make_unique<Light>(LightType::POINT);
	pointLight1->SetPosition(glm::vec3(0.0f, 10.0f, 0.0f));   // 위치: 맵 중앙, 높이 10m
	pointLight1->SetAmbient(glm::vec3(0.1f, 0.1f, 0.1f));     // 주변광: 매우 약한 흰색
	pointLight1->SetDiffuse(glm::vec3(1.0f, 0.9f, 0.8f));     // 확산광: 따뜻한 백색광 (전구 느낌)
	pointLight1->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));    // 반사광: 밝은 하이라이트
	pointLight1->SetIntensity(1.5f);                           // 강도: 150% (1.0 이상 가능)
	// 감쇠 계산식: attenuation = 1.0 / (constant + linear * distance + quadratic * distance²)
	pointLight1->SetAttenuation(1.0f, 0.09f, 0.032f);         // 감쇠: (상수, 선형, 이차) - 약 50m 범위
	pointLight1->SetEnabled(true);
	lights.push_back(std::move(pointLight1));

	// 3. 포인트 조명 2 (Point Light) - 맵 왼쪽 위의 보조 조명
	auto pointLight2 = std::make_unique<Light>(LightType::POINT);
	pointLight2->SetPosition(glm::vec3(-15.0f, 8.0f, -15.0f)); // 위치: 왼쪽 위 코너
	pointLight2->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));      // 주변광: 없음
	pointLight2->SetDiffuse(glm::vec3(0.8f, 0.4f, 0.2f));      // 확산광: 오렌지색 (난로 불빛)
	pointLight2->SetSpecular(glm::vec3(1.0f, 0.6f, 0.4f));     // 반사광: 주황빛 하이라이트
	pointLight2->SetIntensity(1.0f);                            // 강도: 100%
	pointLight2->SetAttenuation(1.0f, 0.14f, 0.07f);           // 감쇠: 약 30m 범위 (더 빨리 감쇠)
	pointLight2->SetEnabled(true);
	lights.push_back(std::move(pointLight2));

	// 4. 포인트 조명 3 (Point Light) - 맵 오른쪽 아래의 청록색 조명
	auto pointLight3 = std::make_unique<Light>(LightType::POINT);
	pointLight3->SetPosition(glm::vec3(15.0f, 6.0f, 15.0f));   // 위치: 오른쪽 아래 코너
	pointLight3->SetAmbient(glm::vec3(0.0f, 0.05f, 0.05f));    // 주변광: 약간의 청록빛
	pointLight3->SetDiffuse(glm::vec3(0.2f, 0.8f, 0.8f));      // 확산광: 밝은 청록색 (네온 느낌)
	pointLight3->SetSpecular(glm::vec3(0.5f, 1.0f, 1.0f));     // 반사광: 밝은 청록빛 하이라이트
	pointLight3->SetIntensity(1.2f);                            // 강도: 120%
	pointLight3->SetAttenuation(1.0f, 0.14f, 0.07f);           // 감쇠: 약 30m 범위
	pointLight3->SetEnabled(true);
	lights.push_back(std::move(pointLight3));

	// 5. 스팟 조명 1 (Spot Light) - 플레이어를 따라가는 손전등
	auto spotLight1 = std::make_unique<Light>(LightType::SPOT);
	spotLight1->SetPosition(playerStartPos + glm::vec3(0.0f, 2.0f, 0.0f)); // 위치: 플레이어 머리 위
	spotLight1->SetDirection(glm::vec3(0.0f, -0.8f, -0.6f));    // 방향: 약간 아래를 향함
	spotLight1->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));        // 주변광: 없음
	spotLight1->SetDiffuse(glm::vec3(1.0f, 1.0f, 0.9f));        // 확산광: 약간 노란빛 흰색
	spotLight1->SetSpecular(glm::vec3(1.0f, 1.0f, 1.0f));       // 반사광: 밝은 하이라이트
	spotLight1->SetIntensity(2.0f);                              // 강도: 200% (강한 손전등)
	spotLight1->SetAttenuation(1.0f, 0.09f, 0.032f);            // 감쇠: 약 50m 범위
	// 스팟라이트 원뿔 각도: cutOff는 내부 원뿔, outerCutOff는 외부 경계
	spotLight1->SetSpotAngle(
		glm::cos(glm::radians(12.5f)),  // cutOff: 내부 각도 12.5도 (밝은 중심부)
		glm::cos(glm::radians(17.5f))   // outerCutOff: 외부 각도 17.5도 (부드러운 경계)
	);
	spotLight1->SetEnabled(true);
	lights.push_back(std::move(spotLight1));

	// 6. 스팟 조명 2 (Spot Light) - 고정된 무대 조명 (빨간색)
	auto spotLight2 = std::make_unique<Light>(LightType::SPOT);
	spotLight2->SetPosition(glm::vec3(-10.0f, 15.0f, 0.0f));    // 위치: 왼쪽 높은 곳
	spotLight2->SetDirection(glm::vec3(0.5f, -1.0f, 0.0f));     // 방향: 오른쪽 아래를 향함
	spotLight2->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));        // 주변광: 없음
	spotLight2->SetDiffuse(glm::vec3(1.0f, 0.1f, 0.1f));        // 확산광: 강한 빨간색
	spotLight2->SetSpecular(glm::vec3(1.0f, 0.5f, 0.5f));       // 반사광: 붉은빛 하이라이트
	spotLight2->SetIntensity(1.5f);                              // 강도: 150%
	spotLight2->SetAttenuation(1.0f, 0.07f, 0.017f);            // 감쇠: 약 70m 범위
	spotLight2->SetSpotAngle(
		glm::cos(glm::radians(20.0f)),  // cutOff: 내부 각도 20도
		glm::cos(glm::radians(25.0f))   // outerCutOff: 외부 각도 25도
	);
	spotLight2->SetEnabled(true);
	lights.push_back(std::move(spotLight2));

	// 7. 포인트 조명 4 (Point Light) - 맵 앞쪽의 보라색 액센트 조명
	auto pointLight4 = std::make_unique<Light>(LightType::POINT);
	pointLight4->SetPosition(glm::vec3(0.0f, 5.0f, -20.0f));    // 위치: 맵 앞쪽 중앙
	pointLight4->SetAmbient(glm::vec3(0.05f, 0.0f, 0.05f));     // 주변광: 약간의 보라빛
	pointLight4->SetDiffuse(glm::vec3(0.6f, 0.2f, 0.8f));       // 확산광: 보라색 (신비로운 느낌)
	pointLight4->SetSpecular(glm::vec3(0.8f, 0.5f, 1.0f));      // 반사광: 밝은 보라빛 하이라이트
	pointLight4->SetIntensity(1.0f);                             // 강도: 100%
	pointLight4->SetAttenuation(1.0f, 0.14f, 0.07f);            // 감쇠: 약 30m 범위
	pointLight4->SetEnabled(true);
	lights.push_back(std::move(pointLight4));

	// 8. 포인트 조명 5 (Point Light) - 맵 뒤쪽의 녹색 조명
	auto pointLight5 = std::make_unique<Light>(LightType::POINT);
	pointLight5->SetPosition(glm::vec3(0.0f, 4.0f, 20.0f));     // 위치: 맵 뒤쪽 중앙
	pointLight5->SetAmbient(glm::vec3(0.0f, 0.05f, 0.0f));      // 주변광: 약간의 녹색
	pointLight5->SetDiffuse(glm::vec3(0.3f, 1.0f, 0.3f));       // 확산광: 밝은 녹색 (출구 표시등 느낌)
	pointLight5->SetSpecular(glm::vec3(0.5f, 1.0f, 0.5f));      // 반사광: 연한 녹색 하이라이트
	pointLight5->SetIntensity(0.8f);                             // 강도: 80%
	pointLight5->SetAttenuation(1.0f, 0.22f, 0.20f);            // 감쇠: 약 20m 범위 (짧은 범위)
	pointLight5->SetEnabled(true);
	lights.push_back(std::move(pointLight5));

	// Renderer에 모든 조명 등록
	if (g_engine) {
		Renderer* renderer = g_engine->GetRenderer();
		if (renderer) {
			// 기존 조명 클리어
			renderer->ClearLights();

			// 모든 조명 추가
			for (auto& lightPtr : lights) {
				renderer->AddLight(lightPtr.get());
			}

			std::cout << "\n===== MULTI-LIGHT SYSTEM INITIALIZED =====" << std::endl;
			std::cout << "Total lights: " << lights.size() << std::endl;
			std::cout << "  - 1 Directional Light (Global sun)" << std::endl;
			std::cout << "  - 5 Point Lights (Various colors)" << std::endl;
			std::cout << "  - 2 Spot Lights (Focused beams)" << std::endl;
			std::cout << "==========================================\n" << std::endl;
		}
	}

	std::cout << "TestScene: Map generation complete" << std::endl;
	std::cout << "TestScene: Initialization complete" << std::endl;
}

void TestScene::Exit()
{
	std::cout << "TestScene: Exited" << std::endl;

	// CollisionManager 정리
	extern Engine* g_engine;
	if (g_engine) {
		CollisionManager* collisionMgr = g_engine->GetCollisionManager();
		if (collisionMgr) {
			collisionMgr->ClearAll();
			std::cout << "TestScene: CollisionManager cleared" << std::endl;
		}

		// InputManager 액션 해제
		InputManager* inputMgr = g_engine->GetInputManager();
		if (inputMgr) {
			inputMgr->ActionW = nullptr;
			inputMgr->ActionS = nullptr;
			inputMgr->ActionA = nullptr;
			inputMgr->ActionD = nullptr;
			inputMgr->ActionSpace = nullptr;
			inputMgr->ActionShift = nullptr;
		}
	}

	player.reset();
	lee.reset();
	light.reset();
	walls.clear();
	floor.reset();
	ceiling.reset();
	mapGenerator.reset();
	lights.clear();
	navMesh.reset();
}

void TestScene::Update(float deltaTime)
{
	// Player가 이동하면 자동으로 카메라를 동기화
	if (player) {
		player->Update(deltaTime);
	}

	if (lee) {
		lee->Update(deltaTime);
	}
	if (floor) {
		floor->Update(deltaTime);
	}
	// ceiling 제거됨
	for (auto& wall : walls) {
		if (wall) {
			wall->Update(deltaTime);
		}
	}
}

void TestScene::Draw()
{
	extern Engine* g_engine;
	if (!g_engine) return;

	Renderer* renderer = g_engine->GetRenderer();
	Camera* camera = g_engine->GetCamera();
	if (!renderer || !camera) return;

	// Light를 Renderer에 설정
	if (light) {
		renderer->SetLight(light.get());
	}

	// 카메라 디버그 출력 (프레임당 한 번씩)
	static int debugFrameCount = 0;
	if (debugFrameCount++ % 120 == 0) {
		std::cout << "\n===== CAMERA DEBUG =====" << std::endl;
		glm::vec3 camPos = camera->GetPosition();
		glm::vec3 camDir = camera->GetDirection();
		std::cout << "Camera Position: (" << camPos.x << ", " << camPos.y << ", " << camPos.z << ")" << std::endl;
		std::cout << "Camera Direction (target): (" << camDir.x << ", " << camDir.y << ", " << camDir.z << ")" << std::endl;
		glm::vec3 viewVec = camDir - camPos;
		std::cout << "View Vector: (" << viewVec.x << ", " << viewVec.y << ", " << viewVec.z << ")" << std::endl;
		if (light) {
			std::cout << "Light Position: (" << light->GetPosition().x << ", " << light->GetPosition().y << ", " << light->GetPosition().z << ")" << std::endl;
		}
		std::cout << "========================\n" << std::endl;
	}

	// Frustum Culling 통계 (디버그용)
	int totalWalls = 0;
	int renderedWalls = 0;

	// 바닥 렌더링 (백페이스 컬링 비활성화)
	static bool floorDebugPrinted = false;
	if (floor && floor->IsActive()) {
		glDisable(GL_CULL_FACE);  // Plane은 양면 렌더링 필요
		glm::mat4 floorMatrix = floor->GetModelMat();
		if (!floorDebugPrinted) {
			std::cout << "TestScene: Rendering floor..." << std::endl;
			std::cout << "  Size: " << floor->GetSize().x << "x" << floor->GetSize().y << std::endl;
			std::cout << "  Position: (" << floor->GetPosition().x << ", " << floor->GetPosition().y << ", " << floor->GetPosition().z << ")" << std::endl;
			std::cout << "  TextureID: " << (floor->GetTextureID().empty() ? "NONE" : floor->GetTextureID()) << std::endl;
			std::cout << "  Color: (" << floor->GetColor().r << ", " << floor->GetColor().g << ", " << floor->GetColor().b << ")" << std::endl;
			floorDebugPrinted = true;
		}
		if (!floor->GetTextureID().empty()) {
			renderer->RenderObjWithTextureTiled("PlaneModel", floor->GetTextureID(), floorMatrix, floor->GetTextureTiling());
		}
		else {
			renderer->RenderObj("PlaneModel", floorMatrix, floor->GetColor());
		}
		glEnable(GL_CULL_FACE);   // 백페이스 컬링 복원
	}
	else {
		static bool floorMissingPrinted = false;
		if (!floorMissingPrinted) {
			std::cerr << "TestScene: Floor is NULL or not active!" << std::endl;
			floorMissingPrinted = true;
		}
	}

	player->DrawHands(renderer);
	// 천장 제거됨 (위에서 내려다보기 위해)
	// 만약 천장을 추가한다면 백페이스 컬링을 활성화 상태로 유지
	// (위에서 내려다보므로 천장 아랫면이 보임 - 백페이스 컬링 필요)

	// 벽 렌더링 (Frustum Culling 임시 비활성화)
	for (const auto& wall : walls) {
		totalWalls++;
		if (wall && wall->IsActive()) {
			// Frustum Culling 체크
			glm::vec3 minBound, maxBound;
			wall->GetBoundingBox(minBound, maxBound);

			// 임시로 항상 렌더링 (frustum culling 비활성화)
			//if (camera->IsBoxInFrustum(minBound, maxBound)) {
				renderedWalls++;
				glm::mat4 wallMatrix = wall->GetModelMat();
				if (!wall->GetTextureID().empty()) {
					renderer->RenderObjWithTexture(wall->GetResourceID(), wall->GetTextureID(), wallMatrix);
				}
				else {
					renderer->RenderObj(wall->GetResourceID(), wallMatrix, wall->GetColor());
				}
			//}
		}
	}

	// Professor (RunLee) 렌더링 - 애니메이션 + 텍스처
	if (lee && lee->IsActive()) {
		glm::mat4 professorMatrix = lee->GetModelMat();

		FBXAnimationPlayer* animPlayer = g_engine->GetAnimationPlayer();
		if (animPlayer && animPlayer->IsPlaying()) {
			renderer->RenderFBXAnimated("RunLee", "RunLee", professorMatrix, animPlayer->GetBoneTransforms());
		}
		else {
			renderer->RenderFBX("RunLee", "RunLee", professorMatrix);
		}
	}

	// 프레임당 한 번만 출력 (60fps라면 60번에 1번)
	static int frameCount = 0;
	if (frameCount++ % 60 == 0) {
		std::cout << "Frustum Culling: " << renderedWalls << " / " << totalWalls << " walls rendered" << std::endl;
	}
}
