#include "MainLevelScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"

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
}

void MainLevelScene::Exit()
{
	_active = false;
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
}

void MainLevelScene::Draw()
{
	if(!_active)
		return;
}