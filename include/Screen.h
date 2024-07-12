#pragma once
#include "raylib.h"

#include <unordered_set>
#include <vector>
#include <string>
#include <utility>

class Services;

#define Tile std::pair<std::string, std::pair<Color, Color>>

struct PairHash {
    template <typename T, typename U>
    std::size_t operator()(const std::pair<T, U> &p) const {
        auto hash1 = std::hash<T>{}(p.first);
        auto hash2 = std::hash<U>{}(p.second);
        return hash1 ^ (hash2 << 1);
    }
};

struct PairEqual {
    template <typename T, typename U>
    bool operator()(const std::pair<T, U> &lhs, const std::pair<T, U> &rhs) const {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }
};	

class Screen
{
private:

	// Ptr of global services
	Services* _services;

	Vector2 _screenSize;
	Rectangle _rec;

	std::vector<std::vector<Tile>> _screen;
	std::unordered_set<std::pair<int, int>, PairHash, PairEqual> _changedTiles;

	Font _font;
	Tile _backgroundTile;

	RenderTexture2D _texture;

	void Init();

public:

	Screen(const Rectangle& rec, const Tile& backgroundTile, const std::string fontPath, const int& fontSize);
	~Screen();

	Vector2 GetScreenSize();
	Tile GetBackgroundTile();

	void Resize(const Rectangle& rec, const int& size);
	void Reset();

	bool ChangeTile(const Tile& tile, const Vector2& position);

	void Draw();
};

bool CompareTile(const Tile& a, const Tile& b);

void DrawTextTile(Screen& screen, const Vector2& start, const std::string& string, const Color& textColor, const Color& backgroundColor);

void DrawCircleTile(Screen& screen, const Vector2& center, const int& radius, const Tile& tile);
void DrawLineTile(Screen& screen, const Vector2& start, const Vector2& end, const Tile& tile);
void DrawRectangleTile(Screen& screen, const Rectangle& rect, const Tile& tile);
void DrawTriangleTile(Screen& screen, const Vector2& point1, const Vector2& point2, const Vector2& point3, const Tile& tile);