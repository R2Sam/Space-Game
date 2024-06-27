#include "GameStateHandler.h"
#include "Services.h"
#include "EventHandler.h"

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
	
}