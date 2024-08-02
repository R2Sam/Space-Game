#include "MainLevelScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"
#include "GameStateHandler.h"

#include "OrbitalSimulation.h"
#include "Screen.h"

#include "MyRaylib.h"
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
	_bodyTile = {"○", {GREEN, LIGHTGRAY}};
	_sunTile =  {"☼", {ORANGE, YELLOW}};
	_moonTile = {"○", {GRAY, LIGHTGRAY}};
	_craftTile = {"•", {RED, LIGHTGRAY}};
	_mapTile = {"♪", {GRAY, DARKGRAY}};
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
	_planets = _services->GetGameStateHandler()->orbitalSimulation->GetCelestialBodies();
	_planetsMap = _services->GetGameStateHandler()->orbitalSimulation->GetCelestialBodiesMap();

	_craft = _services->GetGameStateHandler()->orbitalSimulation->GetOrbitalBodies();
	_craftMap = _services->GetGameStateHandler()->orbitalSimulation->GetOrbitalBodiesMap();
}

void MainLevelScene::DrawMap()
{
	static float scaleFactor = 0.0025;

	Vector2 screenSize = _services->GetGameStateHandler()->screen->GetScreenSize();
	Screen& screen = *_services->GetGameStateHandler()->screen;

	const static Vector2 center = {(screenSize.x / 2), screenSize.y / 2};

	const static Rectangle rec = CenteredRectangle(Rectangle {0, 0, 32, 32}, center);

	Vector3d focus = _planetsMap["Earth"]->position;

	screen.Reset();

	for (std::weak_ptr<OrbitalBody>& ptr : _craft)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = (body->position - focus) * scaleFactor;

		Vector2 pos = {std::round(v.x + center.x), std::round(-v.z + center.y)};

		_services->GetGameStateHandler()->screen->ChangeTile(_craftTile, pos);
	}

	for (CelestialBody* body : _planets)
	{
		Vector3d v = (body->position - focus) * scaleFactor;

		Vector2 pos = {std::round(v.x + center.x), std::round(-v.z + center.y)};

		if (body->name == "Sun")
		{
			if (body->radius * scaleFactor > 1)
			{
				DrawCircleTile(screen, pos, body->radius * scaleFactor, _sunTile);
			}

			else
			{
				screen.ChangeTile(_sunTile, pos);
			}
		}

		else if (body->parent && body->parent->name == "Jupiter")
		{
			if (body->radius * scaleFactor > 1)
			{
				DrawCircleTile(screen, pos, body->radius * scaleFactor, _moonTile);
			}

			else
			{
				screen.ChangeTile(_moonTile, pos);
			}
		}

		else
		{
			if (body->radius * scaleFactor > 1)
			{
				DrawCircleTile(screen, pos, body->radius * scaleFactor, _bodyTile);
			}

			else
			{
				screen.ChangeTile(_bodyTile, pos);
			}
		}
	}

	DrawTextTile(screen, Vector2{0, 0}, "Date:" + _services->GetGameStateHandler()->orbitalSimulation->GetDate(), BLACK, LIGHTGRAY);
	DrawTextTile(screen, Vector2{0, 1}, "Speed:" + std::to_string(_services->GetGameStateHandler()->orbitalSimulation->GetSpeed()), BLACK, LIGHTGRAY);
	DrawTextTile(screen, Vector2{0, 2}, "FPS:" + std::to_string(GetFPS()), BLACK, LIGHTGRAY);

	///*
	std::shared_ptr<OrbitalBody> craft = _craftMap["ISS"].lock();
	//CelestialBody* craft = _planetsMap["ISS"];

	Vector3d position = craft->position - craft->parent->position;
	Vector3d velocity = craft->velocity - craft->parent->velocity;
	double mu = craft->parent->mass * 6.67430e-20;
	Vector3d h = position.cross(velocity);
	Vector3d e = ((velocity.cross(h) / mu) - position.normalize());

	DrawTextTile(screen, Vector2{0, 3}, "ISS Parent:" + craft->parent->name , BLACK, LIGHTGRAY);
	DrawTextTile(screen, Vector2{0, 4}, "ISS Height:" + DoubleToRoundedString(craft->position.distance(craft->parent->position) - craft->parent->radius, 0) + " km" , BLACK, LIGHTGRAY);
	DrawTextTile(screen, Vector2{0, 5}, "ISS Speed:" + DoubleToRoundedString(velocity.length(), 2) + " km/s" , BLACK, LIGHTGRAY);
	DrawTextTile(screen, Vector2{0, 6}, "ISS Eccentricity:" + DoubleToRoundedString(e.length(), 4) , BLACK, LIGHTGRAY);
	//*/
}

void MainLevelScene::Enter()
{
	_active = true;

	_services->GetGameStateHandler()->orbitalSimulation->LoadBodiesFromFile("../data/Bodies.txt");

	_services->GetGameStateHandler()->orbitalSimulation->SetSpeed(100e-1);
}

void MainLevelScene::Exit()
{
	_active = false;

	_services->GetGameStateHandler()->orbitalSimulation->SetSpeed(0);
	_services->GetGameStateHandler()->orbitalSimulation->SaveBodiesToFile("../data/Bodies-Save.txt");
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

	it = _keys.find(KEY_S);
	if (it != _keys.end())
	{
		_services->GetGameStateHandler()->orbitalSimulation->SaveBodiesToFile("../data/Bodies-Save.txt");
	}

	it = _keys.find(KEY_K);
	if (it != _keys.end())
	{
		if (FileExists("../data/Bodies-Save.txt"))
		{
			_services->GetGameStateHandler()->orbitalSimulation->LoadBodiesFromFile("../data/Bodies-Save.txt");
		}

		else
		{
			Log("Bodies-Save does not exist");
		}
	}

	it = _keys.find(KEY_L);
	if (it != _keys.end())
	{
		_services->GetGameStateHandler()->orbitalSimulation->SaveBodiesToFile("../data/Bodies-Save.txt");
		_services->GetGameStateHandler()->orbitalSimulation->LoadBodiesFromFile("../data/Bodies.txt");
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