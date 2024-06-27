#pragma once
#include "raylib.h"
#include <vector>
#include <string>

struct LogWindowInfo
{
	Rectangle windowRec;
	Vector2 pos;
	Color color;
	unsigned int xMargin;
	unsigned int yMargin;

	unsigned int scrollWidth;
	Color scrollColor;
};

class LogWindow
{
private:

	LogWindowInfo _logWindowInfo;
	Rectangle _rectangle;

	Rectangle _scrollRectangle;
	bool _scrolling = false;

	unsigned int _fontSize;
	unsigned int _visibleLinesMax;
	unsigned int  _scrollPosition;

	Font _font;
	Color _fontColor;
	bool _customFont = false;

	int _newMsgFirstLineIndex = -1;
	std::vector<std::string> _messages;
	std::vector<std::string> _logLines;
	std::vector<std::string> _logLinesToShow;

	void Recalculate(const std::vector<std::string> messages);

	void ParseLine(const std::string& line);

	void UpdateScroll();
	void UpdateScrollBar(const Vector2& mousePos , const bool& pressed);

public:

	LogWindow();
	~LogWindow();

	void Init(const LogWindowInfo& logWindowRectangle, const unsigned int& fontSize, const Color& fontColor);
	void Init(const LogWindowInfo& logWindowRectangle, const unsigned int& fontSize, const Color& fontColor, const Font& font);

	void SetRec(const LogWindowInfo& logWindowRectangle);

	void AddMessage(const std::string& message);
	void Clear();

	void Update(const Vector2& mousePos ,const float& mouseScroll, const bool& pressed);
	void Draw();
};