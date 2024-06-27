#include "MenuScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"

#include "Utility.h"

MenuScene::MenuScene(Services* servicesIn) : _services(servicesIn)
{
	AddSelfAsListener();
	Init();
}

MenuScene::~MenuScene()
{
	_services->GetEventHandler()->RemoveListener(_ptr);
}

void MenuScene::Init()
{
	_levelButton = std::make_unique<Button>(false, Rectangle{0, 0, 300, 75}, "Level");
}

void MenuScene::AddSelfAsListener()
{
	_services->GetEventHandler()->AddListener(_ptr);
	_services->GetEventHandler()->AddLocalListener("Menu" ,_ptr);
}

void MenuScene::OnEvent(std::shared_ptr<const Event>& event)
{
	if(!_active)
		return;

}

void MenuScene::GetInputs()
{
	// Get mouse position
	_mousePos = GetMousePosition();

	// Get mouse keys pressed and down
	_keyLeftMousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

	// Get keys pressed
	_keyEscapePressed = IsKeyPressed(KEY_ESCAPE);
}

void MenuScene::Enter()
{
	_active = true;
}

void MenuScene::Exit()
{
	_active = false;
}

void MenuScene::Update()
{
	if(!_active)
		return;

	GetInputs();

	if (_keyEscapePressed)
	{
		std::unique_ptr<const Event> event = std::make_unique<const ClosingEvent>();
		_services->GetEventHandler()->AddLocalEvent("SceneHandler", std::move(event));

		_keyEscapePressed = false;
		_active = false;
		return;
	}

	if (_levelButton->Update(Vector2{_services->screenWidth	* 0.5, _services->screenHeight	* 0.5}, _mousePos, _keyLeftMousePressed))
	{
		std::unique_ptr<const Event> event = std::make_unique<const NextSceneEvent>("MainLevel");
		_services->GetEventHandler()->AddLocalEvent("SceneHandler", std::move(event));

		_keyLeftMousePressed = false;
		return;
	}
}

void MenuScene::Draw()
{
	if(!_active)
		return;

	_levelButton->Draw();
}

