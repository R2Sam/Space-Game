#include "MainLevelScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"
#include "GameStateHandler.h"

#include "OrbitalSimulation.h"
#include "Screen.h"

#include "MyRaylib.h"

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
	_bodyTile = {"█", {BLUE, BLUE}};
	_craftTile = {"•", {RED, WHITE}};
	_mapTile = {"☺", {GRAY, DARKGRAY}};
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
	_mouseKeys.clear();

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

void MainLevelScene::UpdateMap()
{
	_planets = _services->GetGameStateHandler()->orbitalSimulation->GetBodiesV(true);
	_craft = _services->GetGameStateHandler()->orbitalSimulation->GetBodiesV(false);
}

void MainLevelScene::DrawMap()
{
	static const float scaleFactor = 2.0e-08;

	Vector2 screenSize = _services->GetGameStateHandler()->screen->GetScreenSize();
	Screen& screen = *_services->GetGameStateHandler()->screen;

	const static Vector2 center = {(screenSize.x / 2), screenSize.y / 2};

	const static Rectangle rec = CenteredRectangle(Rectangle {0, 0, 32, 32}, center);

	//DrawRectangleTile(screen, rec, _mapTile);
	screen.Reset();

	for (std::weak_ptr<OrbitalBody>& ptr : _craft)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = body->position * scaleFactor;

		Vector2 pos = {std::round(v.x + center.x), std::round(-v.z + center.y)};

		if (body->radius * scaleFactor > 1)
		{
			DrawCircleTile(screen, pos, body->radius * scaleFactor, _craftTile);
		}

		else
		{
			_services->GetGameStateHandler()->screen->ChangeTile(_craftTile, pos);
		}
	}

	for (std::weak_ptr<OrbitalBody>& ptr : _planets)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = body->position * scaleFactor;

		Vector2 pos = {std::round(v.x + center.x), std::round(-v.z + center.y)};

		if (body->radius * scaleFactor > 1)
		{
			DrawCircleTile(screen, pos, body->radius * scaleFactor, _bodyTile);
		}

		else
		{
			_services->GetGameStateHandler()->screen->ChangeTile(_bodyTile, pos);
		}
	}

	DrawTextTile(screen, Vector2{0, 0}, _services->GetGameStateHandler()->orbitalSimulation->GetDate(), BLACK, WHITE);
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

	UpdateMap();
}

void MainLevelScene::Draw()
{
	if(!_active)
		return;

	DrawMap();

	_services->GetGameStateHandler()->screen->Draw();
}