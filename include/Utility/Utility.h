#pragma once
#include "raylib.h"
#include <string>

class Animation
{
private:

	// Texture, base rectangle and amount of frames
	Texture2D _texture;
	Rectangle _rectangle;
	int _rows;
	int _framesNumber;


	int _currentFrameIndex = 0;
	Vector2 _frameGrid = {-1, 0};
	Vector2 _totalFrameGrid;

	// How many game frames per animation frame
	int _frameDuration;
	int _frameCounter = 0;

	// Flags
	bool _running = false;
	bool _once = false;

public:

	Animation(const Texture2D& textureIn, const Rectangle rectangleIn, const int rowsIn, const int frameDurationIn);
	~Animation();

	void Start(const bool& once);
	void Stop();

	void Update();
	bool Draw(const Vector2 pos, const int rot, const float scale);
};

class Button
{
private:

	//  Button box
	Rectangle _rectangle;

	// Center pos;
	Vector2 _pos;

	// True rectangle according to position
	Rectangle _trueRectangle;

	// Can have text and color or texture
	std::string _text;
	Color _textColor = BLACK;
	Color _bodyColor = LIGHTGRAY;
	Color _toggleBodyColor = GRAY;
	Color _edgeColor = GRAY;

	Texture2D _texture;

	// What mode
	bool _textMode;

	// If is a toggle
	bool _toggle = false;

	bool _state = false;

public:

	Button(const bool& toggleIn, const Rectangle& recIn, const std::string& textIn);
	Button(const bool& toggleIn, const Rectangle& recIn, const std::string& textIn, const Color& textColorIn, const Color& bodyColorIn, const Color& toggleBodyColorIn, const Color& edgeColorIn);
	Button(const bool& toggleIn, const Rectangle& recIn, const Texture2D& textureIn);
	~Button();

	bool Update(const Vector2& pos, const Vector2& mousePos, const bool& mouseClick);
	void Draw();
};

class Text
{
private:

	std::string _text;

	Vector2 _pos = {0, 0};

	int _textSize = 20;
	Color _textColor = BLACK;

public:

	Text(const std::string& textIn, const Vector2& posIn);
	Text(const std::string& textIn, const Vector2& posIn, const int& textSizeIn, const Color& textColorIn);
	~Text();

	void Update(const std::string& text, const Vector2& pos);

	void Draw();
};