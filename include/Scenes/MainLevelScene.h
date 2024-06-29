#pragma once
#include "Scene.h"
#include "Event.h"

#include "raylib.h"

#include <unordered_set>
#include <unordered_map>

class Services;

class MainLevelScene : public Scene, public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	// Keys
	std::unordered_set<int> _keys;
	std::unordered_map<int, bool> _keysDown;

	// Mouse
	Vector2 _mousePosition = {0, 0};
	Vector2 _mouseDelta = {0, 0};

	float _mouseScroll = 0;

	std::unordered_map<int, bool> _mouseKeys;
	std::unordered_map<int, bool> _mouseKeysDown;

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