#include "GameStateHandler.h"
#include "Services.h"
#include "EventHandler.h"

#include "OrbitalSimulation.h"
#include "Screen.h"

#include <string>

GameStateHandler::GameStateHandler(Services* servicesIn) : _services(servicesIn)
{
	AddSelfAsListener();
	Init();
}

GameStateHandler::~GameStateHandler()
{
	_services->GetEventHandler()->RemoveListener(_ptr);
}

void GameStateHandler::Init()
{
	orbitalSimulation = std::make_unique<OrbitalSimulation>(_services, 10, true);

	Tile backgroundTile = std::make_pair("█", std::make_pair(LIGHTGRAY, LIGHTGRAY));
	screen = std::make_unique<Screen>(Rectangle{0, 0, _services->screenWidth, _services->screenHeight}, backgroundTile, "../data/Mx437_IBM_EGA_8x8.ttf", 16);
}

void GameStateHandler::AddSelfAsListener()
{
	_services->GetEventHandler()->AddListener(_ptr);
	_services->GetEventHandler()->AddLocalListener("GameStateHandler", _ptr);
}

void GameStateHandler::OnEvent(std::shared_ptr<const Event>& event)
{

}

void GameStateHandler::Update()
{
	orbitalSimulation->Update();
}