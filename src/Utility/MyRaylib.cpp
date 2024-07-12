#include "MyRaylib.h"

#include "raymath.h"
#include "Log.h"

#include <iomanip>
#include <sstream>

void DrawTextureScale(const Texture2D& texture, const Vector2& position, const float& scale, const Color& color)
{
    DrawTexturePro(texture, Rectangle{ 0, 0, texture.width, texture.height },
                Rectangle{ position.x, position.y, texture.width * scale, texture.height * scale},
                Vector2{ texture.width * scale / 2.0f, texture.height * scale / 2.0f },
                0,
                color);
}

void DrawTextureRot(const Texture2D& texture, const Vector2& position, const int& rotation, const Color& color)
{
	DrawTexturePro(texture, Rectangle{ 0, 0, texture.width, texture.height },
                Rectangle{ position.x, position.y, texture.width, texture.height },
                Vector2{ texture.width / 2.0f, texture.height / 2.0f },
                rotation,
                color);
}

void DrawTextureRotScale(const Texture2D& texture, const Vector2& position, const int& rotation, const float& scale, const Color& color)
{
    DrawTexturePro(texture, Rectangle{ 0, 0, texture.width, texture.height },
                Rectangle{ position.x, position.y, texture.width * scale, texture.height * scale},
                Vector2{ texture.width * scale / 2.0f, texture.height * scale / 2.0f },
                rotation,
                color);
}

void DrawTextureRotScaleSelect(const Texture2D& texture, const Rectangle& selection, const Vector2& position, const int& rotation, const float& scale, const Color& color)
{
    DrawTexturePro(texture, selection,
                Rectangle{ position.x, position.y, selection.width * scale, selection.height * scale},
                Vector2{ selection.width * scale / 2.0f, selection.height * scale / 2.0f },
                rotation,
                color);
}

void DrawTextRec(const std::string& text, const int& fontSize, const Color& textColor, const Rectangle& rec, const Color& recColor)
{
    Vector2 textSize = {MeasureText(text.c_str(), fontSize), fontSize};
    Vector2 textPos;

    textPos.x = rec.x + (rec.width / 2) - (textSize.x / 2);
    textPos.y = rec.y + (rec.height / 2) - (textSize.y / 2);

    DrawRectangleRec(rec, recColor);
    DrawText(text.c_str(), textPos.x, textPos.y, fontSize, textColor);
}

Vector2 Angle2Vector(const int& degrees)
{
    float radians = DEG2RAD * degrees;

    return {cos(radians), sin(radians)};
}

int Vector2Angle(const Vector2& vec)
{
    float rad = atan2(vec.y, vec.x);
    rad += PI / 2;

    float deg = rad * RAD2DEG;
    if (deg < 0)
        deg += 360;

    return deg;
}

Vector2 Vector2Rot(const int& length, const int& rotation)
{
    Vector2 result = {0, -1};
    result = Vector2Scale(result, length);
    result = Vector2Rotate(result, rotation * DEG2RAD);
    return result;
}

int DegreeRot(const int rot)
{
    int rotation = rot % 360;

    if (rotation < 0)
        rotation += 360;

    return rotation;
}

int AngleFromPos(const Vector2& pos1, const Vector2& pos2)
{
    Vector2 direction = Vector2Subtract(pos1, pos2);
    int angle = atan2f(direction.x, -direction.y) * RAD2DEG;

    angle -= 180;

    if (angle < 0)
        angle += 360;

    else if (angle >= 360)
        angle -= 360;

    return angle;  
}

std::vector<std::string> WordList(const std::string& input)
{
    std::vector<std::string> words;
    std::istringstream iss(input);
    std::string token;

    while (std::getline(iss, token, ' ') || std::getline(iss, token, '\n'))
    {
        words.push_back(token);
    }

    return words;
}

void DrawTexturePoly(Texture2D texture, Vector2 center, Vector2 *points, Vector2 *texcoords, int pointCount, Color tint)
{
    rlSetTexture(texture.id);

    // Texturing is only supported on RL_QUADS
    rlBegin(RL_QUADS);

        rlColor4ub(tint.r, tint.g, tint.b, tint.a);

        for (int i = 0; i < pointCount - 1; i++)
        {
            rlTexCoord2f(0.5f, 0.5f);
            rlVertex2f(center.x, center.y);

            rlTexCoord2f(texcoords[i].x, texcoords[i].y);
            rlVertex2f(points[i].x + center.x, points[i].y + center.y);

            rlTexCoord2f(texcoords[i + 1].x, texcoords[i + 1].y);
            rlVertex2f(points[i + 1].x + center.x, points[i + 1].y + center.y);

            rlTexCoord2f(texcoords[i + 1].x, texcoords[i + 1].y);
            rlVertex2f(points[i + 1].x + center.x, points[i + 1].y + center.y);
        }
    rlEnd();

    rlSetTexture(0);
}

std::string DoubleToRoundedString(const double& num, const int& precision)
{
    double roundedNum = std::round(num * std::pow(10, precision)) / std::pow(10, precision);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << roundedNum;

    return stream.str();
}

Rectangle CenteredRectangle(const Rectangle& rec, const Vector2& pos)
{
    return Rectangle{pos.x - (rec.width / 2), pos.y - (rec.height / 2), rec.width, rec.height};
}

RenderTexture2D LoadShadowmapRenderTexture(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(0, 0); // Load an empty framebuffer
    target.texture.width = width;
    target.texture.height = height;

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        target.depth.id = rlLoadTextureDepth(width, height, false);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

void UnloadShadowmapRenderTexture(RenderTexture2D target)
{
    if (target.id > 0)
    {
        rlUnloadFramebuffer(target.id);
    }
}

std::chrono::time_point<std::chrono::high_resolution_clock> BeginTimer()
{
    return std::chrono::high_resolution_clock::now();
}

void EndTimer(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime, const std::string& name, const bool& percentage, const int& frames)
{
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - startTime;

    static int i = 0;
    i++;
    if (i >= frames)
    {
        i = 0;
        if (percentage)
        {
            Log("Section " << name << " took: " << (elapsed.count() / GetFrameTime()) * 100 << "%");
        }

        else
        {
            Log("Section " << name << " took: " << elapsed.count() * 1000 << "ms");
        }
    }
}

bool ColorCompare(const Color& a, const Color& b)
{
    if (a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a)
    {
        return true;
    }

    else
    {
        return false;
    }
}

double DateToSeconds(const std::string& dateString, const std::tm& epoch)
{
    std::tm timeinfo = {};
    std::tm ep = epoch;
    std::istringstream ss(dateString);

    ss >> timeinfo.tm_sec;
    ss.ignore(1, ':');
    ss >> timeinfo.tm_min;
    ss.ignore(1, ':');
    ss >> timeinfo.tm_hour;
    ss.ignore(1, ':');
    ss >> timeinfo.tm_mday;
    ss.ignore(1, ':');
    ss >> timeinfo.tm_mon;
    ss.ignore(1, ':');
    ss >> timeinfo.tm_year;

    timeinfo.tm_mon--;
    timeinfo.tm_year -= 1900;
    timeinfo.tm_isdst = -1;

    time_t timeSinceEpoch = mktime(&timeinfo);
    time_t epochSeconds = mktime(&ep);

    if (timeSinceEpoch == -1 || epochSeconds == -1)
    {
        return -1;
    }

    return difftime(timeSinceEpoch, epochSeconds);
}

std::string SecondsToDate(double seconds, const std::tm& epoch)
{
    std::tm ep = epoch;
    time_t epochSeconds = mktime(&ep);

    if (epochSeconds == -1)
    {
        return "";
    }

    time_t requestedTime = epochSeconds + static_cast<time_t>(seconds);
    std::tm* timeinfo = std::localtime(&requestedTime);

    if (timeinfo == nullptr)
    {
        return "";
    }

    timeinfo->tm_mon++;
    timeinfo->tm_year += 1900;

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << timeinfo->tm_sec << ':'
        << std::setfill('0') << std::setw(2) << timeinfo->tm_min << ':'
        << std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ':'
        << std::setfill('0') << std::setw(2) << timeinfo->tm_mday << ':'
        << std::setfill('0') << std::setw(2) << timeinfo->tm_mon << ':'
        << std::setw(4) << timeinfo->tm_year;

    return oss.str();
}