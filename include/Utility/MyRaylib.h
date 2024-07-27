#pragma once
#include "raylib.h"
#include "rlgl.h"

#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <atomic>

// Draw texture with scaling
void DrawTextureScale(const Texture2D& texture, const Vector2& position, const float& scale, const Color& color);
// Draw textures centered on the origin with rotation in degrees
void DrawTextureRot(const Texture2D& texture, const Vector2& position, const int& rotation, const Color& color);
// Same as above but scaling factor aswell
void DrawTextureRotScale(const Texture2D& texture, const Vector2& position, const int& rotation, const float& scale, const Color& color);
// Same as above but you input a rectangle to choose the desired sprite
void DrawTextureRotScaleSelect(const Texture2D& texture, const Rectangle& selection, const Vector2& position, const int& rotation, const float& scale, const Color& color);

// Draw text centered on a rec
void DrawTextRec(const std::string& text, const int& fontSize, const Color& textColor, const Rectangle& rec, const Color& recColor);

// Turn an angle in degrees to a vector
Vector2 Angle2Vector(const int& degrees);
// Get angle of vector
int Vector2Angle(const Vector2& vec);
// Get a vector of certain length and rotation in degrees
Vector2 Vector2Rot(const int& length, const int& rotation);

// Convert from vector rot to bound by 360 degrees
int DegreeRot(const int rot);

// Angle from one pos to another
int AngleFromPos(const Vector2& pos1, const Vector2& pos2);

// String to list of words
std::vector<std::string> WordList(const std::string& input);

// Draw a texture as a polygon of n points with no intersecting edges all visible to the center
void DrawTexturePoly(Texture2D texture, Vector2 center, Vector2 *points, Vector2 *texcoords, int pointCount, Color tint);

// Convert a double to a string with a certain amout of precision
std::string DoubleToRoundedString(const double& num, const int& precision);

// Get a centered rectangle
Rectangle CenteredRectangle(const Rectangle& rec, const Vector2& pos);

// Used for shadowmaps
RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
void UnloadShadowmapRenderTexture(RenderTexture2D target);

// Time a segment of code and log every n frames
std::chrono::time_point<std::chrono::high_resolution_clock> BeginTimer();
void EndTimer(const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime, const std::string& name, const bool& percentage, const int& frames);

// Check if two colors are same
bool ColorCompare(const Color& a, const Color& b);

// Date or seconds from a certain epoch
double DateToSeconds(const std::string& dateString, const std::tm& epoch);
std::string SecondsToDate(double seconds, const std::tm& epoch);

// Thread syncing
void ThreadSync(std::atomic<bool>& start, std::atomic<int>& ready, std::atomic<int>& done, const int& threadNumber);
void ThreadDone(std::atomic<int>& done);
void WaitForThreads(std::atomic<bool>& start, std::atomic<int>& ready, std::atomic<int>& done, const int& threadNumber);

// Vector2 made out of doubles
struct Vector2d
{
    double x, y;

	constexpr inline Vector2d(const double& x = 0, const double& y = 0) : x(x), y(y) {}

	constexpr inline Vector2d operator+(const Vector2d& rhs) const
	{
	    return Vector2d(x + rhs.x, y + rhs.y);
	}

	constexpr inline Vector2d operator-(const Vector2d& rhs) const
	{
	    return Vector2d(x - rhs.x, y - rhs.y);
	}

	constexpr inline Vector2d operator+(const double& scalar) const
	{
	    return Vector2d(x + scalar, y + scalar);
	}

	constexpr inline Vector2d operator-(const double& scalar) const 
	{
	    return Vector2d(x - scalar, y - scalar);
	}

	constexpr inline Vector2d operator*(const Vector2d& rhs) const
	{
	    return Vector2d(x * rhs.x, y * rhs.y);
	}

	constexpr inline Vector2d operator*(const double& scalar) const
	{
	    return Vector2d(x * scalar, y * scalar);
	}

	constexpr inline Vector2d operator/(const double& scalar) const
	{
	    return Vector2d(x / scalar, y / scalar);
	}

	constexpr inline Vector2d operator-() const
	{
	    return Vector2d(-x, -y);
	}

	constexpr inline Vector2d& operator+=(const Vector2d& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    return *this;
	}

	constexpr inline Vector2d& operator-=(const Vector2d& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    return *this;
	}

	constexpr inline Vector2d& operator+=(const double& scalar)
	{
	    x += scalar;
	    y += scalar;
	    return *this;
	}

	constexpr inline Vector2d& operator-=(const double& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    return *this;
	}

	constexpr inline Vector2d& operator*=(const Vector2d& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    return *this;
	}

	constexpr inline Vector2d& operator*=(const double& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    return *this;
	}

	constexpr inline Vector2d& operator/=(const double& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    return *this;
	}

	constexpr inline Vector2 vec2() const
	{
		return Vector2{(float)x, (float)y};
	}

	constexpr inline double dot(const Vector2d& rhs) const
	{
	    return x * rhs.x + y * rhs.y;
	}

	constexpr inline Vector2d cross(const Vector2d& rhs) const
	{
	    return Vector2d(x * rhs.y - y * rhs.x, y * rhs.x - x * rhs.y);
	}

	constexpr inline double length() const
	{
	    return std::sqrt(x * x + y * y);
	}

	constexpr inline double lengthSqr() const
	{
	    return x * x + y * y;
	}

	constexpr inline double distance(const Vector2d& other) const
	{
	    double dx = other.x - x;
	    double dy = other.y - y;
	    return std::sqrt(dx * dx + dy * dy);
	}

	constexpr inline Vector2d normalize() const
	{
	    double len = length();
	    Vector2d  normal = *this;
	    if (len > 0)
	    {
	        normal.x /= len;
	        normal.y /= len;
	    }
	    return normal;
	}
};

constexpr inline Vector2d Vector2dZero()
{
    return Vector2d {0, 0};
}


// Vector3 made out of doubles
struct Vector3d
{
    double x, y, z;

	constexpr inline Vector3d(const double& x = 0, const double& y = 0, const double& z = 0) : x(x), y(y), z(z) {}

	constexpr inline Vector3d operator+(const Vector3d& rhs) const
	{
	    return Vector3d(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	constexpr inline Vector3d operator-(const Vector3d& rhs) const
	{
	    return Vector3d(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	constexpr inline Vector3d operator+(const double& scalar) const
	{
	    return Vector3d(x + scalar, y + scalar, z + scalar);
	}

	constexpr inline Vector3d operator-(const double& scalar) const 
	{
	    return Vector3d(x - scalar, y - scalar, z - scalar);
	}

	constexpr inline Vector3d operator*(const Vector3d& rhs) const
	{
	    return Vector3d(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	constexpr inline Vector3d operator*(const double& scalar) const
	{
	    return Vector3d(x * scalar, y * scalar, z * scalar);
	}

	friend constexpr inline Vector3d operator*(const double scalar, const Vector3d& lhs)
	{
        return Vector3d(lhs.x * scalar, lhs.y * scalar, lhs.z * scalar);
    }

	constexpr inline Vector3d operator/(const double& scalar) const
	{
	    return Vector3d(x / scalar, y / scalar, z / scalar);
	}

	constexpr inline Vector3d operator-() const
	{
	    return Vector3d(-x, -y, -z);
	}

	constexpr inline bool operator== (const Vector3d& rhs) const
	{
	    if (x == rhs.x && y == rhs.y && z == rhs.z)
	    {
	    	return true;
	    }

	    return false;
	}

	constexpr inline bool operator!= (const Vector3d& rhs) const
	{
	    if (x != rhs.x || y != rhs.y || z != rhs.z)
	    {
	    	return true;
	    }

	    return false;
	}

	constexpr inline Vector3d& operator+=(const Vector3d& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    z += rhs.z;
	    return *this;
	}

	constexpr inline Vector3d& operator-=(const Vector3d& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    z -= rhs.z;
	    return *this;
	}

	constexpr inline Vector3d& operator+=(const double& scalar)
	{
	    x += scalar;
	    y += scalar;
	    z += scalar;
	    return *this;
	}

	constexpr inline Vector3d& operator-=(const double& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    z -= scalar;
	    return *this;
	}

	constexpr inline Vector3d& operator*=(const Vector3d& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    z *= rhs.z;
	    return *this;
	}

	constexpr inline Vector3d& operator*=(const double& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    z *= scalar;
	    return *this;
	}

	constexpr inline Vector3d& operator/=(const double& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    z /= scalar;
	    return *this;
	}

	constexpr inline Vector3 vec3() const
	{
		return Vector3{(float)x, (float)y, (float)z};
	}

	constexpr inline double dot(const Vector3d& rhs) const
	{
	    return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	constexpr inline Vector3d cross(const Vector3d& rhs) const
	{
	    return Vector3d(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}

	constexpr inline double length() const
	{
	    return std::sqrt(x * x + y * y + z * z);
	}

	constexpr inline double lengthSqr() const
	{
	    return x * x + y * y + z * z;
	}

	constexpr inline double distance(const Vector3d& other) const
	{
	    double dx = other.x - x;
	    double dy = other.y - y;
	    double dz = other.z - z;
	    return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	constexpr inline Vector3d normalize() const
	{
	    double len = length();
	    Vector3d normal = *this;
	    if (len > 0)
	    {
	        normal.x /= len;
	        normal.y /= len;
	        normal.z /= len;
	    }
	    return normal;
	}
};

constexpr inline Vector3d Vector3dZero()
{
    return Vector3d {0, 0, 0};
}

// Vector2 made out of float
struct Vector2f
{
    float x, y;

	constexpr inline Vector2f(const float& x = 0, const float& y = 0) : x(x), y(y) {}

	constexpr inline Vector2f operator+(const Vector2f& rhs) const
	{
	    return Vector2f(x + rhs.x, y + rhs.y);
	}

	constexpr inline Vector2f operator-(const Vector2f& rhs) const
	{
	    return Vector2f(x - rhs.x, y - rhs.y);
	}

	constexpr inline Vector2f operator+(const float& scalar) const
	{
	    return Vector2f(x + scalar, y + scalar);
	}

	constexpr inline Vector2f operator-(const float& scalar) const 
	{
	    return Vector2f(x - scalar, y - scalar);
	}

	constexpr inline Vector2f operator*(const Vector2f& rhs) const
	{
	    return Vector2f(x * rhs.x, y * rhs.y);
	}

	constexpr inline Vector2f operator*(const float& scalar) const
	{
	    return Vector2f(x * scalar, y * scalar);
	}

	constexpr inline Vector2f operator/(const float& scalar) const
	{
	    return Vector2f(x / scalar, y / scalar);
	}

	constexpr inline Vector2f operator-() const
	{
	    return Vector2f(-x, -y);
	}

	constexpr inline Vector2f& operator+=(const Vector2f& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    return *this;
	}

	constexpr inline Vector2f& operator-=(const Vector2f& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    return *this;
	}

	constexpr inline Vector2f& operator+=(const float& scalar)
	{
	    x += scalar;
	    y += scalar;
	    return *this;
	}

	constexpr inline Vector2f& operator-=(const float& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    return *this;
	}

	constexpr inline Vector2f& operator*=(const Vector2f& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    return *this;
	}

	constexpr inline Vector2f& operator*=(const float& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    return *this;
	}

	constexpr inline Vector2f& operator/=(const float& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    return *this;
	}

	constexpr inline Vector2 vec2() const
	{
		return Vector2{x, y};
	}

	constexpr inline float dot(const Vector2f& rhs) const
	{
	    return x * rhs.x + y * rhs.y;
	}

	constexpr inline Vector2f cross(const Vector2f& rhs) const
	{
	    return Vector2f(x * rhs.y - y * rhs.x, y * rhs.x - x * rhs.y);
	}

	constexpr inline float length() const
	{
	    return std::sqrt(x * x + y * y);
	}

	constexpr inline float lengthSqr() const
	{
	    return x * x + y * y;
	}

	constexpr inline float distance(const Vector2f& other) const
	{
	    float dx = other.x - x;
	    float dy = other.y - y;
	    return std::sqrt(dx * dx + dy * dy);
	}

	constexpr inline Vector2f normalize() const
	{
	    float len = length();
	    Vector2f normal = *this;
	    if (len > 0)
	    {
	        normal.x /= len;
	        normal.y /= len;
	    }
	    return normal;
	}
};

constexpr inline Vector2f Vector2fZero()
{
    return Vector2f {0, 0};
}

// Vector3 made out of floats
struct Vector3f
{
    float x, y, z;

	constexpr inline Vector3f(const float& x = 0, const float& y = 0, const float& z = 0) : x(x), y(y), z(z) {}

	constexpr inline Vector3f operator+(const Vector3f& rhs) const
	{
	    return Vector3f(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	constexpr inline Vector3f operator-(const Vector3f& rhs) const
	{
	    return Vector3f(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	constexpr inline Vector3f operator+(const float& scalar) const
	{
	    return Vector3f(x + scalar, y + scalar, z + scalar);
	}

	constexpr inline Vector3f operator-(const float& scalar) const 
	{
	    return Vector3f(x - scalar, y - scalar, z - scalar);
	}

	constexpr inline Vector3f operator*(const Vector3f& rhs) const
	{
	    return Vector3f(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	constexpr inline Vector3f operator*(const float& scalar) const
	{
	    return Vector3f(x * scalar, y * scalar, z * scalar);
	}

	constexpr inline Vector3f operator/(const float& scalar) const
	{
	    return Vector3f(x / scalar, y / scalar, z / scalar);
	}

	constexpr inline Vector3f operator-() const
	{
	    return Vector3f(-x, -y, -z);
	}

	constexpr inline Vector3f& operator+=(const Vector3f& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    z += rhs.z;
	    return *this;
	}

	constexpr inline Vector3f& operator-=(const Vector3f& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    z -= rhs.z;
	    return *this;
	}

	constexpr inline Vector3f& operator+=(const float& scalar)
	{
	    x += scalar;
	    y += scalar;
	    z += scalar;
	    return *this;
	}

	constexpr inline Vector3f& operator-=(const float& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    z -= scalar;
	    return *this;
	}

	constexpr inline Vector3f& operator*=(const Vector3f& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    z *= rhs.z;
	    return *this;
	}

	constexpr inline Vector3f& operator*=(const float& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    z *= scalar;
	    return *this;
	}

	constexpr inline Vector3f& operator/=(const float& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    z /= scalar;
	    return *this;
	}

	constexpr inline Vector3 vec3() const
	{
		return Vector3{x, y, z};
	}

	constexpr inline float dot(const Vector3f& rhs) const
	{
	    return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	constexpr inline Vector3f cross(const Vector3f& rhs) const
	{
	    return Vector3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}

	constexpr inline float length() const
	{
	    return std::sqrt(x * x + y * y + z * z);
	}

	constexpr inline float lengthSqr() const
	{
	    return x * x + y * y + z * z;
	}

	constexpr inline float distance(const Vector3f& other) const
	{
	    float dx = other.x - x;
	    float dy = other.y - y;
	    float dz = other.z - z;
	    return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	constexpr inline Vector3f normalize() const
	{
	    float len = length();
	    Vector3f normal = *this;
	    if (len > 0)
	    {
	        normal.x /= len;
	        normal.y /= len;
	        normal.z /= len;
	    }
	    return normal;
	}
};

constexpr inline Vector3f Vector3fZero()
{
    return Vector3f {0, 0, 0};
}