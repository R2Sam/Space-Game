#include "LogWindow.h"

#include <algorithm>

#include "MyRaylib.h"

LogWindow::LogWindow()
{

}

LogWindow::~LogWindow()
{

}

void LogWindow::Init(const LogWindowInfo& logWindowInfo, const unsigned int& fontSize, const Color& fontColor)
{
	_logWindowInfo = logWindowInfo;
	_rectangle = CenteredRectangle(_logWindowInfo.windowRec, _logWindowInfo.pos);

	_scrollRectangle = {0, 0, _logWindowInfo.scrollWidth, _rectangle.height};
	UpdateScroll();

	_fontSize = fontSize;
	_fontColor = fontColor;

	SetTextLineSpacing(_fontSize);
	_visibleLinesMax = (_logWindowInfo.windowRec.height - _logWindowInfo.yMargin * 2) / _fontSize;
}

void LogWindow::Init(const LogWindowInfo& logWindowInfo, const unsigned int& fontSize, const Color& fontColor, const Font& font)
{
	_logWindowInfo = logWindowInfo;
	_rectangle = CenteredRectangle(_logWindowInfo.windowRec, _logWindowInfo.pos);

	_scrollRectangle = {0, 0, _logWindowInfo.scrollWidth, _rectangle.height};
	UpdateScroll();

	_fontSize = fontSize;
	_fontColor = fontColor;
	_font = font;
	_customFont = true;

	SetTextLineSpacing(_fontSize);
	_visibleLinesMax = (_logWindowInfo.windowRec.height - _logWindowInfo.yMargin * 2) / _fontSize;
}

void LogWindow::SetRec(const LogWindowInfo& logWindowInfo)
{
	_logWindowInfo = logWindowInfo;
	_rectangle = CenteredRectangle(_logWindowInfo.windowRec, _logWindowInfo.pos);

	_visibleLinesMax = (_logWindowInfo.windowRec.height - _logWindowInfo.yMargin * 2) / _fontSize;

	Recalculate(_messages);
}

void LogWindow::Recalculate(const std::vector<std::string> messages)
{
	Clear();

	for (const std::string& message : messages)
	{
		AddMessage(message);
	}
}

void LogWindow::ParseLine(const std::string& line)
{
	std::vector<std::string> words = WordList(line);
	
	std::string currentLine;

	for (const std::string& word : words)
	{
		std::string potentialLine = currentLine + word + " ";

		if (!_customFont)
		{
			if (MeasureText(potentialLine.c_str(), _fontSize) > _rectangle.width - _logWindowInfo.xMargin - _logWindowInfo.scrollWidth)
			{
				_logLines.push_back(currentLine);
				currentLine = word + " ";
			}

			else
			{
				currentLine = potentialLine;
			}
		}

		else if (MeasureTextEx( _font, potentialLine.c_str(), _fontSize, _fontSize).x > _rectangle.width - _logWindowInfo.xMargin - _logWindowInfo.scrollWidth)
		{
			_logLines.push_back(currentLine);
			currentLine = word + " ";
		}

		else
		{
			currentLine = potentialLine;
		}
	}

	if (!currentLine.empty())
	{
		_logLines.push_back(currentLine);
	}
}

void LogWindow::UpdateScroll()
{
	float scrollBarHeight = _rectangle.height;

	scrollBarHeight *= _visibleLinesMax / std::max((float)_logLines.size(), (float)_visibleLinesMax);
	scrollBarHeight = std::max(scrollBarHeight, (float)10);

	std::pair<float, float> scrollRange = std::make_pair(_rectangle.y + scrollBarHeight / 2, _rectangle.y + _rectangle.height - scrollBarHeight / 2);

	float scrollY = _rectangle.y + _rectangle.height / 2;

	if (scrollBarHeight != _rectangle.height)
	{
		scrollY = (scrollRange.second - scrollRange.first) * ((float)(_scrollPosition - _visibleLinesMax) / (float)(_logLines.size() - _visibleLinesMax)) + scrollRange.first;
	}

	Vector2 scrollPos = {_rectangle.x + _rectangle.width - (_logWindowInfo.scrollWidth / 2), scrollY};
	_scrollRectangle = CenteredRectangle(Rectangle{0, 0, _logWindowInfo.scrollWidth, scrollBarHeight}, scrollPos);
}

void LogWindow::UpdateScrollBar(const Vector2& mousePos, const bool& pressed)
{
	float scrollBarHeight = _rectangle.height;

	scrollBarHeight *= _visibleLinesMax / std::max((float)_logLines.size(), (float)_visibleLinesMax);
	scrollBarHeight = std::max(scrollBarHeight, (float)10);

	std::pair<float, float> scrollRange = std::make_pair(_rectangle.y + scrollBarHeight / 2, _rectangle.y + _rectangle.height - scrollBarHeight / 2);

	float scrollY = _rectangle.y + _rectangle.height / 2;

	if (scrollBarHeight != _rectangle.height)
	{
		scrollY = (scrollRange.second - scrollRange.first) * ((float)(_scrollPosition - _visibleLinesMax) / (float)(_logLines.size() - _visibleLinesMax)) + scrollRange.first;
	}

	if (CheckCollisionPointRec(mousePos, _scrollRectangle) && pressed)
	{
		_scrolling = true;
	}

	if (_scrolling && scrollBarHeight != _rectangle.height)
	{
		scrollY = std::max(std::min(mousePos.y, scrollRange.second), scrollRange.first);

		unsigned int newScrollPos = ((_logLines.size() - _visibleLinesMax) * (scrollY - scrollRange.first) / (scrollRange.second - scrollRange.first)) + _visibleLinesMax;
		scrollY = (scrollRange.second - scrollRange.first) * ((float)(newScrollPos - _visibleLinesMax) / (float)(_logLines.size() - _visibleLinesMax)) + scrollRange.first;

		if (newScrollPos < _scrollPosition)
		{
			for (int i = 0; i < (_scrollPosition - newScrollPos); i++)
			{
				_scrollPosition--;

				_logLinesToShow.insert(_logLinesToShow.begin(), _logLines[_scrollPosition - _visibleLinesMax]);
				_logLinesToShow.pop_back();
				_logLinesToShow.shrink_to_fit();
			}
		}

		else if (newScrollPos > _scrollPosition)
		{
			for (int i = 0; i < (newScrollPos - _scrollPosition); i++)
			{
				_scrollPosition++;

				_logLinesToShow.push_back(_logLines[_scrollPosition - 1]);
				_logLinesToShow.erase(_logLinesToShow.begin());
				_logLinesToShow.shrink_to_fit();
			}
		}
	}

	Vector2 scrollPos = {_rectangle.x + _rectangle.width - (_logWindowInfo.scrollWidth / 2), scrollY};
	_scrollRectangle = CenteredRectangle(Rectangle{0, 0, _logWindowInfo.scrollWidth, scrollBarHeight}, scrollPos);
}

void LogWindow::AddMessage(const std::string& message)
{
	_messages.push_back(message);

	std::string line;

	if (_newMsgFirstLineIndex != -1)
	{
		_logLines[_newMsgFirstLineIndex].erase(0, 1);
	}
	_newMsgFirstLineIndex = _logLines.size();

	for (const char& c : message)
	{
		line += c;

		if (c == '\n')
		{
			line.pop_back();
			ParseLine(line);

			line.clear();
		}
	}

	if (!line.empty())
	{
		ParseLine(line);
	}

	_logLines[_newMsgFirstLineIndex].insert(0, 1, '>');

	if (_logLines.size() > _visibleLinesMax)
	{
		_logLinesToShow.clear();
		_logLinesToShow.shrink_to_fit();

		for (int i = _logLines.size() - _visibleLinesMax; i < _logLines.size(); i++)
		{
			_logLinesToShow.push_back(_logLines[i]);
		}
	}

	else
	{
		_logLinesToShow = _logLines;
	}

	_scrollPosition = _logLines.size();

	UpdateScroll();
}

void LogWindow::Clear()
{
	_messages.clear();
	_messages.shrink_to_fit();
	_logLines.clear();
	_logLines.shrink_to_fit();
	_logLinesToShow.clear();
	_logLinesToShow.shrink_to_fit();

	UpdateScroll();
}

void LogWindow::Update(const Vector2& mousePos ,const float& mouseScroll, const bool& pressed)
{
	if (CheckCollisionPointRec(mousePos, _rectangle) && mouseScroll != 0 && _logLines.size() > _logLinesToShow.size())
	{
		unsigned int newScrollPos = _scrollPosition;

		if (mouseScroll < 0)
		{
			newScrollPos++;
			newScrollPos = std::clamp(newScrollPos,  _visibleLinesMax, (unsigned int)_logLines.size());

			if (newScrollPos != _scrollPosition)
			{
				_scrollPosition = newScrollPos;

				_logLinesToShow.push_back(_logLines[_scrollPosition - 1]);
				_logLinesToShow.erase(_logLinesToShow.begin());
				_logLinesToShow.shrink_to_fit();
			}
		}

		else
		{
			newScrollPos--;
			newScrollPos = std::clamp(newScrollPos,  _visibleLinesMax, (unsigned int)_logLines.size());

			if (newScrollPos != _scrollPosition)
			{
				_scrollPosition = newScrollPos;

				_logLinesToShow.insert(_logLinesToShow.begin(), _logLines[_scrollPosition - _visibleLinesMax]);
				_logLinesToShow.pop_back();
				_logLinesToShow.shrink_to_fit();
			}
		}

		UpdateScroll();
	}

	if (pressed)
	{
		UpdateScrollBar(mousePos, pressed);
	}

	if (_scrolling && !pressed)
	{
		_scrolling = false;
	}
}

void LogWindow::Draw()
{
	std::string totalText;

	for (const std::string& line : _logLinesToShow)
	{
		totalText += line + " \n";
	}

	DrawRectangleRec(_rectangle, _logWindowInfo.color);

	DrawRectangleRec(_scrollRectangle, _logWindowInfo.scrollColor);

	if (!_customFont)
	{
		DrawText(totalText.c_str(), _rectangle.x + _logWindowInfo.xMargin, _rectangle.y + _logWindowInfo.xMargin, _fontSize, _fontColor);
	}
	
	else
	{
		DrawTextEx(_font, totalText.c_str(), Vector2{_rectangle.x + _logWindowInfo.xMargin, _rectangle.y + _logWindowInfo.xMargin}, _fontSize, _fontSize, _fontColor);
	}
}