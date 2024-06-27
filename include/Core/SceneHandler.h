#pragma once
#include "Event.h"
#include "Scene.h"
#include <memory>
#include <unordered_map>
#include <string>

class Services;

class SceneHandler : public EventListener
{
private:

	// Ptr of global services
	Services* _services;

	// Map of all scenes known
	std::unordered_map<std::string, std::unique_ptr<Scene>> _scenes;
    std::unique_ptr<Scene> _currentScene;
    std::string _currentSceneName;

    // Next scene name and if it's been changed to
    std::string _nextSceneName;
    bool _nextSceneCalled = false;

    // Closing
    bool _shouldClose = false;

    void Init();

    void AddSelfAsListener() override;
    void OnEvent(std::shared_ptr<const Event>& event) override;

	void AddScene(const std::string& sceneName, std::unique_ptr<Scene>& scene);
    void RemoveScene(const std::string& sceneName);

    void ChangeScene(const std::string& sceneName);

public:

	SceneHandler(Services* servicesIn);
	~SceneHandler();

    void Update();
    void Draw();
};

class NextSceneEvent : public Event
{
public:

	std::string nextSceneName;

	NextSceneEvent(std::string nextSceneNameIn) : nextSceneName(nextSceneNameIn) {}
};

class ClosingEvent : public Event
{

};