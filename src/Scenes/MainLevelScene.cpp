#include "MainLevelScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"

#include "raylib.h"	

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
	// Get keys pressed
	_keyEscapePressed = IsKeyPressed(KEY_ESCAPE);
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
	if (_keyEscapePressed && _active)
	{
		std::unique_ptr<const Event> event = std::make_unique<const NextSceneEvent>("Menu");
		_services->GetEventHandler()->AddLocalEvent("SceneHandler", std::move(event));

		_keyEscapePressed = false;
		_active = false;
		return;
	}
}

void MainLevelScene::Draw()
{
	if(!_active)
		return;
}