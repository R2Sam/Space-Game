#include "Screen.h"

#include "MyRaylib.h"

#include "raymath.h"

#include <cmath>

Screen::Screen(const Rectangle& rec, const Tile& backgroundTile, const std::string fontPath, const int& fontSize) : _rec(rec), _backgroundTile(backgroundTile)
{
	int count;
	int* points = LoadCodepoints("☺☻♥♦♣♠•◘○◙♂♀♪♫☼►◄↕‼¶§▬↨↑↓→←∟↔▲▼!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~⌂ÇüéâäàåçêëèïîìÄÅÉæÆôöòûùÿÖÜ¢£¥₧ƒáíóúñÑªº¿⌐¬½¼¡«»░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀ɑϐᴦᴨ∑ơµᴛɸϴΩẟ∞∅∈∩≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ", &count);

	_font = LoadFontEx("../data/Mx437_IBM_EGA_8x8.ttf", fontSize, points, count);

	UnloadCodepoints(points);

    _texture = LoadRenderTexture(_rec.width, _rec.height);

	Init();
}

Screen::~Screen() 
{
	UnloadFont(_font);
	UnloadRenderTexture(_texture);
}

void Screen::Init() 
{
	_screenSize.x = _rec.width / _font.baseSize;
    _screenSize.y = _rec.height / _font.baseSize;

	_screen.reserve(_screenSize.x);
    _changedTiles.reserve(_screenSize.x * _screenSize.y);

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

Tile Screen::GetBackgroundTile() 
{
    return _backgroundTile;
}

void Screen::Resize(const Rectangle& rec, const int& size) 
{
    if (size <= 0)
    {
        return;
    }

    _rec = rec;
    _font.baseSize = size;

    _screenSize.x = _rec.width / _font.baseSize;
    _screenSize.y = _rec.height / _font.baseSize;

    _screen.clear();
    _screen.reserve(_screenSize.x);
    _screen.shrink_to_fit();

    _changedTiles.clear();
    _changedTiles.reserve(_screenSize.x * _screenSize.y);

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

void Screen::Reset()
{
    BeginTextureMode(_texture);

    for (int x = 0; x < _screenSize.x; x++)
    {
        for (int y = 0; y < _screenSize.y; y++)
        {
            if (!CompareTile(_screen[x][y], _backgroundTile))
            {
                _screen[x][y] = _backgroundTile;
                DrawRectangle(x * _font.baseSize, y * _font.baseSize, _font.baseSize, _font.baseSize, _screen[x][y].second.second);
                DrawTextEx(_font, _screen[x][y].first.c_str(), Vector2{x * _font.baseSize, y * _font.baseSize}, _font.baseSize, 0, _screen[x][y].second.first);
            }
        }
    }

    EndTextureMode();
}

bool Screen::ChangeTile(const Tile& tile, const Vector2& position) 
{
	if (position.x >= 0 && position.x < _screenSize.x && position.y >= 0 && position.y < _screenSize.y)
	{
        if (!CompareTile(_screen[position.x][position.y], tile))
        {
    		_screen[position.x][position.y] = tile;
    		_changedTiles.emplace(position.x, position.y);

            return true;
        }
	}

	return false;
}

void Screen::Draw() 
{
	BeginTextureMode(_texture);

	for (const std::pair<int, int>& pair : _changedTiles)
	{
		DrawRectangle(pair.first * _font.baseSize, pair.second * _font.baseSize, _font.baseSize, _font.baseSize, _screen[pair.first][pair.second].second.second);
		DrawTextEx(_font, _screen[pair.first][pair.second].first.c_str(), Vector2{pair.first * _font.baseSize, pair.second * _font.baseSize}, _font.baseSize, 0, _screen[pair.first][pair.second].second.first);
	}

	EndTextureMode();

	_changedTiles.clear();

	DrawTextureRec(_texture.texture, Rectangle{0, 0, _texture.texture.width, -_texture.texture.height}, Vector2{_rec.x, _rec.y}, WHITE);
}

bool CompareTile(const Tile& a, const Tile& b)
{
    if (a.first == b.first &&
        ColorCompare(a.second.first, b.second.first) &&
        ColorCompare(a.second.second, b.second.second))
    {
        return true;
    }

    else
    {
        return false;
    }
}

void DrawTextTile(Screen& screen, const Vector2& start, const std::string& string, const Color& textColor, const Color& backgroundColor)
{
    if (string.empty())
    {
        return;
    }

    Tile tile;
    tile.second.first = textColor;
    tile.second.second = backgroundColor;

    Vector2 pos = start;

    for (int i = 0; i < string.size(); i++)
    {
        if (string[i] == '\n')
        {
            pos.y += 1;
        }

        tile.first = string[i];
        screen.ChangeTile(tile, pos);

        pos.x += 1;
    }
}

void DrawCircleTile(Screen& screen, const Vector2& center, const int& radius, const Tile& tile)
{
    if (radius < 1)
    {
        return;
    }

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

void DrawLineTile(Screen& screen, const Vector2& start, const Vector2& end, const Tile& tile)
{
    if (Vector2Distance(start, end) < 1)
    {
        return;
    }

    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        screen.ChangeTile(tile, {x0, y0});

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void DrawRectangleTile(Screen& screen, const Rectangle& rect, const Tile& tile)
{
    if (rect.width < 1 || rect.height < 1)
    {
        return;
    }

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
    if (Vector2Distance(point1, point2) < 1 || Vector2Distance(point1, point3) < 1 || Vector2Distance(point2, point3) < 1)
    {
        return;
    }

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
