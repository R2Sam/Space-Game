#pragma once
#include "Event.h"

class Services;

class GameStateHandler : public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	void AddSelfAsListener() override;
	void OnEvent(std::shared_ptr<const Event>& event) override;

public:

	GameStateHandler(Services* servicesIn);
	~GameStateHandler();

	void Init();

	void Update();
};