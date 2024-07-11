#pragma once
#include "raylib.h"

#include <vector>
#include <string>

class Services;

#define Tile std::pair<std::string, std::pair<Color, Color>>	

class Screen
{
private:

	// Ptr of global services
	Services* _services;

	Vector2 _screenSize;

	std::vector<std::vector<Tile>> _screen;
	std::vector<Vector2> _changedTiles;

	Font _font;
	Tile _backgroundTile;

	RenderTexture2D _texture;

	void Init();

public:

	Screen(Services* services, const Tile& backgroundTile, const std::string fontPath, const int& fontSize);
	~Screen();

	Vector2 GetScreenSize();

	void Reset();

	bool ChangeTile(const Tile& tile, const Vector2& position);

	void Draw();
};

void DrawCircleTile(Screen& screen, const Vector2& center, const int& radius, const Tile& tile);
void DrawRectangleTile(Screen& screen, const Rectangle& rect, const Tile& tile);
void DrawTriangleTile(Screen& screen, const Vector2& point1, const Vector2& point2, const Vector2& point3, const Tile& tile);