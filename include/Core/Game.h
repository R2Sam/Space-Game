#pragma once
#include "Event.h"
#include "Services.h"
#include <string>

class SceneHandler;

class Game : public EventListener
{
private:

	void Init();
	void DeInit();

	void AddSelfAsListener() override;

	// Run once per frame
	void Tick();

	void Update();
	void Draw();

	void OnEvent(std::shared_ptr<const Event>& event) override;

	bool GameShouldClose();

	bool _shouldClose = false;
	
	// Basic game components 
	Services _services;
	Services* _servicesPtr;
	std::unique_ptr<SceneHandler> _sceneHandler;

public:

	Game(int width, int height, const std::string title);
	~Game();

	void Run();
};

class GameCloseEvent : public Event
{
	
};