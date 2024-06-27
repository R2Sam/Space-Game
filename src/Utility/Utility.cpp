#include "Utility.h"

#include "MyRaylib.h"
#include "Log.h"

Animation::Animation(const Texture2D& textureIn, const Rectangle rectangleIn, const int rowsIn, const int frameDurationIn) : _texture(textureIn), _rectangle(rectangleIn), _rows(rowsIn), _frameDuration(frameDurationIn)
{
	_texture = textureIn;

	_framesNumber = _texture.width / _texture.height * _rows * _rows;

	_totalFrameGrid = {_texture.width / _texture.height * _rows, _rows - 1};
}

Animation::~Animation()
{

}

void Animation::Start(const bool& once)
{
	_once = once;
	_running = true;
}

void Animation::Stop()
{
	_running = false;
}

void Animation::Update()
{
	if (_running)
	{
		++_frameCounter;

		if (_frameCounter >= _frameDuration)
		{
			_currentFrameIndex = (_currentFrameIndex + 1) % _framesNumber;

			if (_frameGrid.x >= _totalFrameGrid.x && _frameGrid.y >= _totalFrameGrid.y)
			{
				_frameGrid = {0, 0};

				if (_once)
				{
					_running = false;
				}
			}

			else if (_frameGrid.x >= _totalFrameGrid.x )
			{
				_frameGrid.x = 0;
				_frameGrid.y += 1;
			}

			else
				_frameGrid.x += 1;

			_frameCounter = 0;
		}
	}
}

bool Animation::Draw(const Vector2 pos, const int rot, const float scale)
{
	if (_running)
	{
		Rectangle rec = _rectangle;

		rec.x = rec.width * _frameGrid.x;
		rec.y = rec.height * _frameGrid.y;

		DrawTextureRotScaleSelect(_texture, rec, pos, rot, scale, WHITE);
	}

	return _running;
}

Button::Button(const bool& toggleIn, const Rectangle& recIn, const std::string& textIn) : _toggle(toggleIn), _rectangle(recIn), _text(textIn)
{
	_textMode = true;
}

Button::Button(const bool& toggleIn, const Rectangle& recIn, const std::string& textIn, const Color& textColorIn, const Color& bodyColorIn, const Color& toggleBodyColorIn, const Color& edgeColorIn) : _toggle(toggleIn), _rectangle(recIn), _text(textIn), _textColor(textColorIn), _bodyColor(bodyColorIn), _toggleBodyColor(toggleBodyColorIn), _edgeColor(edgeColorIn) 
{
	_textMode = true;
}

Button::Button(const bool& toggleIn, const Rectangle& recIn, const Texture2D& textureIn) : _toggle(toggleIn), _rectangle(recIn), _texture(textureIn)
{
	_textMode = false;
}

Button::~Button()
{

}

bool Button::Update(const Vector2& pos, const Vector2& mousePos, const bool& mouseClick)
{
	_pos = pos;

	_trueRectangle = CenteredRectangle(_rectangle, _pos);

	if (!_toggle)
	{
		_state = CheckCollisionPointRec(mousePos, _trueRectangle) && mouseClick;
	}

	else
	{
		if (CheckCollisionPointRec(mousePos, _trueRectangle) && mouseClick)
		{
			_state = !_state;
		}
	}

	return _state;
}

void Button::Draw()
{
	DrawRectangleRec(_trueRectangle, _edgeColor);

	if (_textMode)
	{
		Rectangle bodyRec = {_trueRectangle.x + 5, _trueRectangle.y + 5, _trueRectangle.width - 10, _trueRectangle.height - 10};

		if (!_state)
		{
			DrawTextRec(_text, 20, _textColor, bodyRec, _bodyColor);
		}

		else
		{
			DrawTextRec(_text, 20, _textColor, bodyRec, _toggleBodyColor);
		}
	}

	else if (IsTextureReady(_texture))
	{
		DrawTextureScale(_texture, _pos, 1.0, WHITE);
	}

	else
	{
		LogColor("No button texture!", LOG_RED);
	}
}

Text::Text(const std::string& textIn, const Vector2& posIn) : _text(textIn), _pos(posIn)
{

}

Text::Text(const std::string& textIn, const Vector2& posIn, const int& textSizeIn, const Color& textColorIn) : _text(textIn), _pos(posIn), _textSize(textSizeIn), _textColor(textColorIn)
{

}

Text::~Text()
{

}

void Text::Update(const std::string& text, const Vector2& pos)
{
	_text = text;
	_pos = pos;
}

void Text::Draw()
{
	DrawText(_text.c_str(), _pos.x, _pos.y, _textSize, _textColor);
}