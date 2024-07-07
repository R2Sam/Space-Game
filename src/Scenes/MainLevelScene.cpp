#include "MainLevelScene.h"
#include "Services.h"
#include "EventHandler.h"
#include "SceneHandler.h"
#include "GameStateHandler.h"

#include "OrbitalSimulation.h"

#include "MyRaylib.h"

MainLevelScene::MainLevelScene(Services* servicesIn) : _services(servicesIn)
{
	AddSelfAsListener();
	Init();
}

MainLevelScene::~MainLevelScene()
{
	_services->GetEventHandler()->RemoveListener(_ptr);

	UnloadFont(_font);
}

void MainLevelScene::Init()
{
	int count;
	int* points = LoadCodepoints("☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀ɑϐᴦᴨ∑ơµᴛɸϴΩẟ∞∅∈∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ", &count);

	_font = LoadFontEx("../data/Mx437_IBM_EGA_8x8.ttf", 8, points, count);
	SetTextLineSpacing(0);

	UnloadCodepoints(points);

	_fontSize = Vector2{_font.baseSize, _font.baseSize};
	_backgroundTile = std::make_pair("\u2588", std::make_pair(WHITE, WHITE));

	_bodyTile = std::make_pair("\u2588", std::make_pair(BLUE, BLUE));
	_craftTile = std::make_pair("\u25A0", std::make_pair(RED, YELLOW));

	_screenSize.x = _services->screenWidth / _fontSize.x;
	_screenSize.y = _services->screenHeight / _fontSize.y;

	for (int x = 0; x < _screenSize.x; x++)
	{
		std::vector<Tile> column;

		for (int y = 0; y < _screenSize.y; y++)
		{
			column.push_back(_backgroundTile);
		}

		_screen.push_back(column);
	}
}

void MainLevelScene::AddSelfAsListener()
{
	// Groups to listen to
	_services->GetEventHandler()->AddListener(_ptr);
	_services->GetEventHandler()->AddLocalListener("MainLevel" ,_ptr);
}

void MainLevelScene::OnEvent(std::shared_ptr<const Event>& event)
{
	// If not active ignore events
	if (!_active)
		return;
}

void MainLevelScene::GetInputs()
{
	_keys.clear();
	_keysDown.clear();
	_mouseKeys.clear();
	_mouseKeysDown.clear();

	// Get keys pressed
	while (int i = GetKeyPressed())
	{
		_keys.insert(i);
	}

	// Get keys down
	for (auto& pair : _keysDown)
	{
		pair.second = IsKeyDown(pair.first);
	}

	// Get mouse keys
	for (auto& pair : _mouseKeys)
	{
		pair.second = IsMouseButtonPressed(pair.first);
	}

	// Get mouse keys down
	for (auto& pair : _mouseKeysDown)
	{
		pair.second = IsMouseButtonDown(pair.first);
	}

	_mousePosition = GetMousePosition();
	_mouseDelta = GetMouseDelta();

	_mouseScroll = GetMouseWheelMove();
}

void MainLevelScene::ResetScreen()
{
	for (int x = 0; x < _screenSize.x; x++)
	{
		for (int y = 0; y < _screenSize.y; y++)
		{
			_screen[x][y] = _backgroundTile;
		}
	}
}

void MainLevelScene::Enter()
{
	_active = true;

	_services->GetGameStateHandler()->orbitalSimulation->LoadBodiesFromFile("../data/bodies.txt");
	_services->GetGameStateHandler()->orbitalSimulation->SetSpeed(100000);
}

void MainLevelScene::Exit()
{
	_active = false;

	_services->GetGameStateHandler()->orbitalSimulation->SetSpeed(0);
}

void MainLevelScene::Update()
{
	if(!_active)
		return;

	GetInputs();

	// Exit scene and go to menu
	auto it = _keys.find(KEY_ESCAPE);
	if (it != _keys.end() && _active)
	{
		std::unique_ptr<const Event> event = std::make_unique<const NextSceneEvent>("Menu");
		_services->GetEventHandler()->AddLocalEvent("SceneHandler", std::move(event));

		_active = false;
		return;
	}

	_planets = _services->GetGameStateHandler()->orbitalSimulation->GetBodiesV(true);
	_craft = _services->GetGameStateHandler()->orbitalSimulation->GetBodiesV(false);
}

void MainLevelScene::Draw()
{
	if(!_active)
		return;

	ClearBackground(BLACK);

	ResetScreen();

	static const float scaleFactor = 1.4e-05;

	for (std::weak_ptr<OrbitalBody>& ptr : _planets)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = body->position * scaleFactor;

		Vector2 pos = {std::round(v.x + _screenSize.x / 2), std::round(-v.z + _screenSize.y / 2)};

		DrawCircleScreen(_screen ,pos, body->radius * scaleFactor, _bodyTile);
	}

	for (std::weak_ptr<OrbitalBody>& ptr : _craft)
	{
		std::shared_ptr<OrbitalBody> body = ptr.lock();

		if (!body)
		{
			continue;
		}

		Vector3d v = body->position * scaleFactor;

		Vector2 pos = {std::round(v.x + _screenSize.x / 2), std::round(-v.z + _screenSize.y / 2)};

		if (body->radius * scaleFactor >= 1)
		{
			DrawCircleScreen(_screen ,pos, body->radius * scaleFactor, _craftTile);
		}
	}

	for (int x = 0; x < _screen.size(); x++)
	{
		for (int y = 0; y < _screen[x].size(); y++)
		{
			if (_screen[x][y].first != "\u2588")
			{
				DrawRectangle(x * _fontSize.x, y * _fontSize.y, _fontSize.x, _fontSize.y, _screen[x][y].second.second);
			}

			DrawTextEx(_font, _screen[x][y].first.c_str(), Vector2{x * _fontSize.x, y * _fontSize.y}, _fontSize.y, 0, _screen[x][y].second.first);
		}
	}
}

void DrawCircleScreen(std::vector<std::vector<Tile>>& screen, const Vector2& center, const int& radius, const Tile& tile)
{
    int x = radius;
    int y = 0;
    int decisionOver2 = 1 - x;

    std::vector<Vector2> pixels;
    
    while (y <= x)
    {
       	
      	pixels.push_back(Vector2{center.x + x, center.y + y});
	    pixels.push_back(Vector2{center.x + y, center.y + x});
	    pixels.push_back(Vector2{center.x - y, center.y + x});
	    pixels.push_back(Vector2{center.x - x, center.y + y});
	    pixels.push_back(Vector2{center.x - x, center.y - y});
	    pixels.push_back(Vector2{center.x - y, center.y - x});
	    pixels.push_back(Vector2{center.x + y, center.y - x});
	    pixels.push_back(Vector2{center.x + x, center.y - y});
        
        y++;
        
        if (decisionOver2 <= 0)
        {
            decisionOver2 += 2 * y + 1;
        }
        else
        {
            x--;
            decisionOver2 += 2 * (y - x) + 1;
        }
    }

	for (const Vector2& v : pixels)
	{
		if (v.x >= 0 && v.y >= 0 && v.x < screen.size() && v.y < screen[0].size())
		{
			for (int i = center.x; i <= v.x; i++)
            {
                if (i >= 0 && i < screen.size() && v.y >= 0 && v.y < screen[0].size())
                {
                    screen[i][v.y] = tile;
                }
            }

            for (int i = v.x; i <= center.x; i++)
            {
                if (i >= 0 && i < screen.size() && v.y >= 0 && v.y < screen[0].size())
                {
                    screen[i][v.y] = tile;
                }
            }
		}
	}
}