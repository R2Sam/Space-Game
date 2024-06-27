#pragma once
#include "Scene.h"
#include "Event.h"

class Services;

class MainLevelScene : public Scene, public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	// Keys
	bool _keyEscapePressed = false;

	void Init() override;

	void AddSelfAsListener() override;
	void OnEvent(std::shared_ptr<const Event>& event) override;

	void GetInputs() override;

public:

	MainLevelScene(Services* servicesIn);
	~MainLevelScene();

	void Enter() override;
	void Exit() override;

	void Update() override;
	void Draw() override;
};