#pragma once
#include "Scene.h"
#include "Event.h"

#include "raylib.h"	

class Services;

class Button;

class MenuScene : public Scene, public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	// Keys
	bool _keyEscapePressed = false;

	// Mouse
	Vector2 _mousePos = {0, 0};
	bool _keyLeftMousePressed = false;

	// Button
	std::unique_ptr<Button> _levelButton;

	void Init() override;

	void AddSelfAsListener() override;
	void OnEvent(std::shared_ptr<const Event>& event) override;

	void GetInputs() override;

public:

	MenuScene(Services* servicesIn);
	~MenuScene();

	void Enter() override;
	void Exit() override;

	void Update() override;
	void Draw() override;
};