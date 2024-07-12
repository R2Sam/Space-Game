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
	orbitalSimulation = std::make_unique<OrbitalSimulation>(_services, 1, true);
	orbitalSimulation->LoadBodiesFromFile("../data/bodies.txt");

	Tile backgroundTile = std::make_pair("\u2588", std::make_pair(WHITE, WHITE));
	screen = std::make_unique<Screen>(_services, backgroundTile, "../data/Mx437_IBM_EGA_8x8.ttf", 8);
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