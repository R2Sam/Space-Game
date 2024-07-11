#include "Screen.h"

#include "Services.h"

Screen::Screen(Services* services, const Tile& backgroundTile, const std::string fontPath, const int& fontSize) : _services(services), _backgroundTile(backgroundTile)
{
	int count;
	int* points = LoadCodepoints("☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀ɑϐᴦᴨ∑ơµᴛɸϴΩẟ∞∅∈∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ", &count);

	_font = LoadFontEx("../data/Mx437_IBM_EGA_8x8.ttf", fontSize, points, count);
	SetTextLineSpacing(0);

	UnloadCodepoints(points);

	_texture = LoadRenderTexture(_services->screenWidth, _services->screenHeight);

	Init();
}

Screen::~Screen() 
{
	UnloadFont(_font);
	UnloadRenderTexture(_texture);
}

void Screen::Init() 
{
	_screenSize.x = _services->screenWidth / _font.baseSize;
	_screenSize.y = _services->screenHeight / _font.baseSize;

	_screen.reserve(_screenSize.x);

	for (int i = 0; i < _screenSize.x; i++)
	{
		_screen.push_back(std::vector<Tile>{});
		_screen[i].reserve(_screenSize.y);

		for (int j = 0; j < _screenSize.y; j++)
		{
			_screen[i].push_back(_backgroundTile);
		}
	}

	BeginTextureMode(_texture);

	ClearBackground(WHITE);

	for (int x = 0; x < _screenSize.x; x++)
	{
		for (int y = 0; y < _screenSize.y; y++)
		{
			DrawRectangle(x * _font.baseSize, y * _font.baseSize, _font.baseSize, _font.baseSize, _screen[x][y].second.second);
			DrawTextEx(_font, _screen[x][y].first.c_str(), Vector2{x * _font.baseSize, y * _font.baseSize}, _font.baseSize, 0, _screen[x][y].second.first);
		}
	}

	EndTextureMode();
}

Vector2 Screen::GetScreenSize() 
{
	return _screenSize;
}

void Screen::Reset()
{
	for (int x = 0; x < _screenSize.x; x++)
	{
		for (int y = 0; y < _screenSize.y; y++)
		{
			_screen[x][y] = _backgroundTile;
		}
	}
}

bool Screen::ChangeTile(const Tile& tile, const Vector2& position) 
{
	if (position.x >= 0 && position.x <= _screenSize.x && position.y >= 0 && position.y <= _screenSize.y)
	{
		_screen[position.x][position.y] = tile;
		_changedTiles.push_back(position);
	}

	return false;
}

void Screen::Draw() 
{
	BeginTextureMode(_texture);

	for (const Vector2& vec : _changedTiles)
	{
		DrawRectangle(vec.x * _font.baseSize, vec.y * _font.baseSize, _font.baseSize, _font.baseSize, _screen[vec.x][vec.y].second.second);
		DrawTextEx(_font, _screen[vec.x][vec.y].first.c_str(), Vector2{vec.x * _font.baseSize, vec.y * _font.baseSize}, _font.baseSize, 0, _screen[vec.x][vec.y].second.first);
	}

	EndTextureMode();

	_changedTiles.clear();

	DrawTextureRec(_texture.texture, Rectangle{0, 0, _texture.texture.width, -_texture.texture.height}, Vector2{0, 0}, WHITE);
}

void DrawCircleTile(Screen& screen, const Vector2& center, const int& radius, const Tile& tile)
{
    int x = radius;
    int y = 0;
    int decisionOver2 = 1 - x;
    
    while (y <= x)
    {
       	
      	screen.ChangeTile(tile, Vector2{center.x + x, center.y + y});
	    screen.ChangeTile(tile, Vector2{center.x + y, center.y + x});
	    screen.ChangeTile(tile, Vector2{center.x - y, center.y + x});
	    screen.ChangeTile(tile, Vector2{center.x - x, center.y + y});
	    screen.ChangeTile(tile, Vector2{center.x - x, center.y - y});
	    screen.ChangeTile(tile, Vector2{center.x - y, center.y - x});
	    screen.ChangeTile(tile, Vector2{center.x + y, center.y - x});
	    screen.ChangeTile(tile, Vector2{center.x + x, center.y - y});
        
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
}

void DrawRectangleTile(Screen& screen, const Rectangle& rect, const Tile& tile)
{
    for (int x = rect.x; x < rect.x + rect.width; x++)
    {
        for (int y = rect.y; y < rect.y + rect.height; y++)
        {
            screen.ChangeTile(tile, Vector2{x, y});
        }
    }
}

void DrawTriangleTile(Screen& screen, const Vector2& point1, const Vector2& point2, const Vector2& point3, const Tile& tile)
{
	Vector2 p1 = point1;
	Vector2 p2 = point2;
	Vector2 p3 = point3;

    if (p1.y > p2.y)
    {
        Vector2 temp = p1;
        p1 = p2;
        p2 = temp;
    }
    if (p1.y > p3.y)
    {
        Vector2 temp = p1;
        p1 = p3;
        p3 = temp;
    }
    if (p2.y > p3.y)
    {
        Vector2 temp = p2;
        p2 = p3;
        p3 = temp;
    }

    float dx1, dx2, dx3;
    if (p2.y - p1.y > 0)
    {
        dx1 = (p2.x - p1.x) / (p2.y - p1.y);
    }

    else
    {
        dx1 = 0;
    }

    if (p3.y - p1.y > 0)
    {
        dx2 = (p3.x - p1.x) / (p3.y - p1.y);
    }

    else
    {
        dx2 = 0;
    }

    if (p3.y - p2.y > 0)
    {
        dx3 = (p3.x - p2.x) / (p3.y - p2.y);
    }

    else
    {
        dx3 = 0;
    }

    float x1 = p1.x;
    float x2 = p1.x;

    for (int y = p1.y; y <= p2.y; y++)
    {
        for (int x = static_cast<int>(x1); x <= static_cast<int>(x2); x++)
        {
            screen.ChangeTile(tile, Vector2{x, y});
        }
        x1 += dx1;
        x2 += dx2;
    }

    x1 = p2.x;
    for (int y = p2.y; y <= p3.y; y++)
    {
        for (int x = static_cast<int>(x1); x <= static_cast<int>(x2); x++)
        {
            screen.ChangeTile(tile, Vector2{x, y});
        }
        x1 += dx3;
        x2 += dx2;
    }
}
