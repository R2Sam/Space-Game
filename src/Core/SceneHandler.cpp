#include "Game.h"
#include "SceneHandler.h"
#include "Services.h"
#include "EventHandler.h"

#include "MenuScene.h"
#include "MainLevelScene.h"

#include "Log.h"

SceneHandler::SceneHandler(Services* servicesIn) : _services(servicesIn)
{
	AddSelfAsListener();
	Init();
}

SceneHandler::~SceneHandler()
{
	_services->GetEventHandler()->RemoveListener(_ptr);
}

void SceneHandler::Init()
{
	// Create the scenes
	
	std::unique_ptr<Scene> scene = std::make_unique<MenuScene>(_services);
	AddScene("Menu", scene);

	scene = std::make_unique<MainLevelScene>(_services);
	AddScene("MainLevel", scene);

	// Make the first scene
	_currentSceneName = "Menu";

	auto newScene = _scenes.find(_currentSceneName);
    if (newScene != _scenes.end())
    {
    	_currentScene = std::move(newScene->second);
    	_currentScene->Enter();

    	LogColor("Entering scene: " << _currentSceneName, LOG_BLUE);
    }

    else
    	LogColor("Starting scene does not exist: " << _currentSceneName, LOG_RED);

}

void SceneHandler::AddSelfAsListener()
{
	_services->GetEventHandler()->AddListener(_ptr);
	_services->GetEventHandler()->AddLocalListener("SceneHandler" ,_ptr);
}

void SceneHandler::OnEvent(std::shared_ptr<const Event>& event)
{
	// Check if scene change has been called
	if (std::shared_ptr<const NextSceneEvent> nextSceneEvent = std::dynamic_pointer_cast<const NextSceneEvent>(event))
	{
		// Take the name and set flag
		_nextSceneName = nextSceneEvent->nextSceneName;
		_nextSceneCalled = true;
	}

	if (std::shared_ptr<const ClosingEvent> closingEvent = std::dynamic_pointer_cast<const ClosingEvent>(event))
	{
		_shouldClose = true;
	}
}

void SceneHandler::AddScene(const std::string& sceneName, std::unique_ptr<Scene>& scene)
{
    _scenes[sceneName] = std::move(scene);
}

void SceneHandler::RemoveScene(const std::string& sceneName)
{
	// Remove scene from map
    _scenes.erase(sceneName);
}

void SceneHandler::ChangeScene(const std::string& sceneName)
{
	// Check if scene exists
    auto newScene = _scenes.find(sceneName);
    if (newScene != _scenes.end())
    {
    	// Run exit on the current scene
    	_currentScene->Exit();
    	LogColor("Exiting scene: " << _currentSceneName, LOG_BLUE);

    	// Move scene back
    	auto oldscene = _scenes.find(_currentSceneName);
    	oldscene->second = std::move(_currentScene);

    	// Moves the new scene in
        _currentScene = std::move(newScene->second);

        // Run enter on the new current scene
        _currentScene->Enter();
        LogColor("Entering scene: " << sceneName, LOG_BLUE);

        _currentSceneName = sceneName;
    }
    
    else
    	LogColor("Scene does not exist: " << sceneName, LOG_RED);
}

void SceneHandler::Update()
{
	// Check if we should close from window
	if (!_shouldClose)
		_shouldClose = WindowShouldClose();
	
	// Check if should close and exit and delete all scenes
	if (_shouldClose)
	{
		LogColor("Closing scenes", LOG_YELLOW);

		auto oldscene = _scenes.find(_currentSceneName);
    	oldscene->second = std::move(_currentScene);

		for (auto& pair : _scenes)
		{
			if (pair.second)
			{
				LogColor("Closing scene: " << pair.first, LOG_YELLOW);
				pair.second->Exit();
				pair.second.reset();
			}
		}

		std::unique_ptr<const Event> event = std::make_unique<const GameCloseEvent>();
		_services->GetEventHandler()->AddLocalEvent("Game", std::move(event));

		return;
	}

	// Check if scene change is needed
	if (_nextSceneCalled)
	{
		ChangeScene(_nextSceneName);
		_nextSceneCalled = false;
	}

	// Update current scene
    if (_currentScene)
    {
        _currentScene->Update();
    }
}

void SceneHandler::Draw()
{
	// Draw current scene
    if (_currentScene)
    {
        _currentScene->Draw();
    }
}
