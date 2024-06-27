#pragma once
#include <memory>

class EventHandler;
class GameStateHandler;

class Services
{
private:

	void Init();

	// Event, input and game handler objects
	std::unique_ptr<EventHandler> _eventHandler;
	std::unique_ptr<GameStateHandler> _gameStateHandler;

public:

	Services();
	~Services();

	void UpdateVar();
	void UpdateObj();

	EventHandler* GetEventHandler();
	GameStateHandler* GetGameStateHandler();

	bool closeGame = false;

	// Delta Time or time bewteen last frames
	float deltaT;

	int screenWidth;
	int screenHeight;
};