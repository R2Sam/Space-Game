#include "MainLevelScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"
#include "GameStateHandler.h"

#include "OrbitalSimulation.h"

#include "Log.h"

MainLevelScene::MainLevelScene(Services* servicesIn) : _services(servicesIn)
{
	AddSelfAsListener();
	Init();
}

MainLevelScene::~MainLevelScene()
{
	_services->GetEventHandler()->RemoveListener(_ptr);
}

void MainLevelScene::Init()
{
	
}

void MainLevelScene::AddSelfAsListener()
{
	// Groups to listen to
	_services->GetEventHandler()->AddListener(_ptr);
	_services->GetEventHandler()->AddLocalListener("MainLevel" ,_ptr);
}

void MainLevelScene::OnEvent(std::shared_ptr<const Event>& event)
{
	// If not active ignore events
	if (!_active)
		return;
}

void MainLevelScene::GetInputs()
{
	_keys.clear();
	_keysDown.clear();
	_mouseKeys.clear();
	_mouseKeysDown.clear();

	// Get keys pressed
	while (int i = GetKeyPressed())
	{
		_keys.insert(i);
	}

	// Get keys down
	for (auto& pair : _keysDown)
	{
		pair.second = IsKeyDown(pair.first);
	}

	// Get mouse keys
	for (auto& pair : _mouseKeys)
	{
		pair.second = IsMouseButtonPressed(pair.first);
	}

	// Get mouse keys down
	for (auto& pair : _mouseKeysDown)
	{
		pair.second = IsMouseButtonDown(pair.first);
	}

	_mousePosition = GetMousePosition();
	_mouseDelta = GetMouseDelta();

	_mouseScroll = GetMouseWheelMove();
}

void MainLevelScene::Enter()
{
	_active = true;

	_services->GetGameStateHandler()->orbitalSimulation->LoadBodiesFromFile("../data/bodies.txt");
	_services->GetGameStateHandler()->orbitalSimulation->SetSpeed(100000);
}

void MainLevelScene::Exit()
{
	_active = false;

	_services->GetGameStateHandler()->orbitalSimulation->SetSpeed(0);
}

void MainLevelScene::Update()
{
	if(!_active)
		return;

	GetInputs();

	// Exit scene and go to menu
	auto it = _keys.find(KEY_ESCAPE);
	if (it != _keys.end() && _active)
	{
		std::unique_ptr<const Event> event = std::make_unique<const NextSceneEvent>("Menu");
		_services->GetEventHandler()->AddLocalEvent("SceneHandler", std::move(event));

		_active = false;
		return;
	}

	_planets = _services->GetGameStateHandler()->orbitalSimulation->GetBodiesV(true);
	_craft = _services->GetGameStateHandler()->orbitalSimulation->GetBodiesV(false);
}

void MainLevelScene::Draw()
{
	if(!_active)
		return;

	static const float scaleFactor = 0.001;

	for (std::weak_ptr<OrbitalBody>& ptr : _planets)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = body->position * scaleFactor;

		Vector2 pos = {v.x + _services->screenWidth / 2, -v.z + _services->screenHeight / 2};

		DrawCircleV(pos, body->radius * scaleFactor, BLUE);
	}

	for (std::weak_ptr<OrbitalBody>& ptr : _craft)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = body->position * scaleFactor;

		Vector2 pos = {v.x + _services->screenWidth / 2, -v.z + _services->screenHeight / 2};

		DrawCircleV(pos, body->radius * scaleFactor, RED);
	}
}