#include "Game.h"
#include "EventHandler.h"
#include "SceneHandler.h"

#include "raylib.h"

#include "Log.h"

Game::Game(int width, int height, const std::string title)
{
	// Set window flags and create window
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetWindowState( FLAG_VSYNC_HINT | FLAG_WINDOW_ALWAYS_RUN);
	
	InitWindow(width, height, title.c_str());
	SetExitKey(KEY_NULL);
	
	Init();
	AddSelfAsListener();
}

Game::~Game()
{
	DeInit();
	CloseWindow();
}

void Game::Init()
{
	// Basic game handlers
	_servicesPtr = &_services;
	_sceneHandler = std::make_unique<SceneHandler>(_servicesPtr);
}

void Game::DeInit()
{
	_services.GetEventHandler()->RemoveListener(_ptr);
}

void Game::AddSelfAsListener()
{
	_services.GetEventHandler()->AddListener(_ptr);
	_services.GetEventHandler()->AddLocalListener("Game" ,_ptr);
}

void Game::Run()
{
	// Main Game loop
	while (!GameShouldClose())
    {
        Tick();
    }
}

void Game::Tick()
{
	Update();

	BeginDrawing();
	Draw();
	EndDrawing();
}

void Game::Update()
{
	// Update the deltaTime and game window sizes
	_services.UpdateVar();

	// Update current scene
	_sceneHandler->Update();

	// Update service objects
	_services.UpdateObj();
}

void Game::Draw()
{
	// Clear screen to white
	ClearBackground(WHITE);

	// Update current scene
	_sceneHandler->Draw();
}

void Game::OnEvent(std::shared_ptr<const Event>& event)
{
	if (std::shared_ptr<const GameCloseEvent> gameCloseEvent = std::dynamic_pointer_cast<const GameCloseEvent>(event))
	{
		_shouldClose = true;
		LogColor("CLOSING", LOG_YELLOW);
	}
}

bool Game::GameShouldClose()
{
	// Check if window should close
	return _shouldClose;
}