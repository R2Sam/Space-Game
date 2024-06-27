#pragma once
#include "raylib.h"
#include "rlgl.h"

#include <string>
#include <vector>
#include <cmath>

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

// Vector2 made out of doubles
struct Vector2d
{
    double x, y;

	constexpr Vector2d(const double& x = 0, const double& y = 0) : x(x), y(y) {}

	constexpr Vector2d operator+(const Vector2d& rhs) const
	{
	    return Vector2d(x + rhs.x, y + rhs.y);
	}

	constexpr Vector2d operator-(const Vector2d& rhs) const
	{
	    return Vector2d(x - rhs.x, y - rhs.y);
	}

	constexpr Vector2d operator+(const double& scalar) const
	{
	    return Vector2d(x + scalar, y + scalar);
	}

	constexpr Vector2d operator-(const double& scalar) const 
	{
	    return Vector2d(x - scalar, y - scalar);
	}

	constexpr Vector2d operator*(const Vector2d& rhs) const
	{
	    return Vector2d(x * rhs.x, y * rhs.y);
	}

	constexpr Vector2d operator*(const double& scalar) const
	{
	    return Vector2d(x * scalar, y * scalar);
	}

	constexpr Vector2d operator/(const double& scalar) const
	{
	    return Vector2d(x / scalar, y / scalar);
	}

	constexpr Vector2d operator-() const
	{
	    return Vector2d(-x, -y);
	}

	constexpr Vector2d& operator+=(const Vector2d& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    return *this;
	}

	constexpr Vector2d& operator-=(const Vector2d& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    return *this;
	}

	constexpr Vector2d& operator+=(const double& scalar)
	{
	    x += scalar;
	    y += scalar;
	    return *this;
	}

	constexpr Vector2d& operator-=(const double& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    return *this;
	}

	constexpr Vector2d& operator*=(const Vector2d& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    return *this;
	}

	constexpr Vector2d& operator*=(const double& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    return *this;
	}

	constexpr Vector2d& operator/=(const double& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    return *this;
	}

	constexpr Vector2 vec2() const
	{
		return Vector2{(float)x, (float)y};
	}

	constexpr double dot(const Vector2d& rhs) const
	{
	    return x * rhs.x + y * rhs.y;
	}

	constexpr Vector2d cross(const Vector2d& rhs) const
	{
	    return Vector2d(x * rhs.y - y * rhs.x, y * rhs.x - x * rhs.y);
	}

	constexpr double length() const
	{
	    return std::sqrt(x * x + y * y);
	}

	constexpr double lengthSqr() const
	{
	    return x * x + y * y;
	}

	constexpr double distance(const Vector2d& other) const
	{
	    double dx = other.x - x;
	    double dy = other.y - y;
	    return std::sqrt(dx * dx + dy * dy);
	}

	constexpr Vector2d& normalize()
	{
	    double len = length();
	    if (len > 0)
	    {
	        x /= len;
	        y /= len;
	    }
	    return *this;
	}
};

constexpr Vector2d Vector2dZero()
{
    return Vector2d {0, 0};
}


// Vector3 made out of doubles
struct Vector3d
{
    double x, y, z;

	constexpr Vector3d(const double& x = 0, const double& y = 0, const double& z = 0) : x(x), y(y), z(z) {}

	constexpr Vector3d operator+(const Vector3d& rhs) const
	{
	    return Vector3d(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	constexpr Vector3d operator-(const Vector3d& rhs) const
	{
	    return Vector3d(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	constexpr Vector3d operator+(const double& scalar) const
	{
	    return Vector3d(x + scalar, y + scalar, z + scalar);
	}

	constexpr Vector3d operator-(const double& scalar) const 
	{
	    return Vector3d(x - scalar, y - scalar, z - scalar);
	}

	constexpr Vector3d operator*(const Vector3d& rhs) const
	{
	    return Vector3d(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	constexpr Vector3d operator*(const double& scalar) const
	{
	    return Vector3d(x * scalar, y * scalar, z * scalar);
	}

	friend constexpr Vector3d operator*(const double scalar, const Vector3d& lhs)
	{
        return Vector3d(lhs.x * scalar, lhs.y * scalar, lhs.z * scalar);
    }

	constexpr Vector3d operator/(const double& scalar) const
	{
	    return Vector3d(x / scalar, y / scalar, z / scalar);
	}

	constexpr Vector3d operator-() const
	{
	    return Vector3d(-x, -y, -z);
	}

	constexpr bool operator== (const Vector3d& rhs) const
	{
	    if (x == rhs.x && y == rhs.y && z == rhs.z)
	    {
	    	return true;
	    }

	    return false;
	}

	constexpr bool operator!= (const Vector3d& rhs) const
	{
	    if (x != rhs.x || y != rhs.y || z != rhs.z)
	    {
	    	return true;
	    }

	    return false;
	}

	constexpr Vector3d& operator+=(const Vector3d& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    z += rhs.z;
	    return *this;
	}

	constexpr Vector3d& operator-=(const Vector3d& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    z -= rhs.z;
	    return *this;
	}

	constexpr Vector3d& operator+=(const double& scalar)
	{
	    x += scalar;
	    y += scalar;
	    z += scalar;
	    return *this;
	}

	constexpr Vector3d& operator-=(const double& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    z -= scalar;
	    return *this;
	}

	constexpr Vector3d& operator*=(const Vector3d& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    z *= rhs.z;
	    return *this;
	}

	constexpr Vector3d& operator*=(const double& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    z *= scalar;
	    return *this;
	}

	constexpr Vector3d& operator/=(const double& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    z /= scalar;
	    return *this;
	}

	constexpr Vector3 vec3() const
	{
		return Vector3{(float)x, (float)y, (float)z};
	}

	constexpr double dot(const Vector3d& rhs) const
	{
	    return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	constexpr Vector3d cross(const Vector3d& rhs) const
	{
	    return Vector3d(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}

	constexpr double length() const
	{
	    return std::sqrt(x * x + y * y + z * z);
	}

	constexpr double lengthSqr() const
	{
	    return x * x + y * y + z * z;
	}

	constexpr double distance(const Vector3d& other) const
	{
	    double dx = other.x - x;
	    double dy = other.y - y;
	    double dz = other.z - z;
	    return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	constexpr Vector3d& normalize()
	{
	    double len = length();
	    if (len > 0)
	    {
	        x /= len;
	        y /= len;
	        z /= len;
	    }
	    return *this;
	}
};

constexpr Vector3d Vector3dZero()
{
    return Vector3d {0, 0, 0};
}

// Vector2 made out of float
struct Vector2f
{
    float x, y;

	constexpr Vector2f(const float& x = 0, const float& y = 0) : x(x), y(y) {}

	constexpr Vector2f operator+(const Vector2f& rhs) const
	{
	    return Vector2f(x + rhs.x, y + rhs.y);
	}

	constexpr Vector2f operator-(const Vector2f& rhs) const
	{
	    return Vector2f(x - rhs.x, y - rhs.y);
	}

	constexpr Vector2f operator+(const float& scalar) const
	{
	    return Vector2f(x + scalar, y + scalar);
	}

	constexpr Vector2f operator-(const float& scalar) const 
	{
	    return Vector2f(x - scalar, y - scalar);
	}

	constexpr Vector2f operator*(const Vector2f& rhs) const
	{
	    return Vector2f(x * rhs.x, y * rhs.y);
	}

	constexpr Vector2f operator*(const float& scalar) const
	{
	    return Vector2f(x * scalar, y * scalar);
	}

	constexpr Vector2f operator/(const float& scalar) const
	{
	    return Vector2f(x / scalar, y / scalar);
	}

	constexpr Vector2f operator-() const
	{
	    return Vector2f(-x, -y);
	}

	constexpr Vector2f& operator+=(const Vector2f& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    return *this;
	}

	constexpr Vector2f& operator-=(const Vector2f& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    return *this;
	}

	constexpr Vector2f& operator+=(const float& scalar)
	{
	    x += scalar;
	    y += scalar;
	    return *this;
	}

	constexpr Vector2f& operator-=(const float& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    return *this;
	}

	constexpr Vector2f& operator*=(const Vector2f& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    return *this;
	}

	constexpr Vector2f& operator*=(const float& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    return *this;
	}

	constexpr Vector2f& operator/=(const float& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    return *this;
	}

	constexpr Vector2 vec2() const
	{
		return Vector2{x, y};
	}

	constexpr float dot(const Vector2f& rhs) const
	{
	    return x * rhs.x + y * rhs.y;
	}

	constexpr Vector2f cross(const Vector2f& rhs) const
	{
	    return Vector2f(x * rhs.y - y * rhs.x, y * rhs.x - x * rhs.y);
	}

	constexpr float length() const
	{
	    return std::sqrt(x * x + y * y);
	}

	constexpr float lengthSqr() const
	{
	    return x * x + y * y;
	}

	constexpr float distance(const Vector2f& other) const
	{
	    float dx = other.x - x;
	    float dy = other.y - y;
	    return std::sqrt(dx * dx + dy * dy);
	}

	constexpr Vector2f& normalize()
	{
	    float len = length();
	    if (len > 0)
	    {
	        x /= len;
	        y /= len;
	    }
	    return *this;
	}
};

constexpr Vector2f Vector2fZero()
{
    return Vector2f {0, 0};
}

// Vector3 made out of floats
struct Vector3f
{
    float x, y, z;

	constexpr Vector3f(const float& x = 0, const float& y = 0, const float& z = 0) : x(x), y(y), z(z) {}

	constexpr Vector3f operator+(const Vector3f& rhs) const
	{
	    return Vector3f(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	constexpr Vector3f operator-(const Vector3f& rhs) const
	{
	    return Vector3f(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	constexpr Vector3f operator+(const float& scalar) const
	{
	    return Vector3f(x + scalar, y + scalar, z + scalar);
	}

	constexpr Vector3f operator-(const float& scalar) const 
	{
	    return Vector3f(x - scalar, y - scalar, z - scalar);
	}

	constexpr Vector3f operator*(const Vector3f& rhs) const
	{
	    return Vector3f(x * rhs.x, y * rhs.y, z * rhs.z);
	}

	constexpr Vector3f operator*(const float& scalar) const
	{
	    return Vector3f(x * scalar, y * scalar, z * scalar);
	}

	constexpr Vector3f operator/(const float& scalar) const
	{
	    return Vector3f(x / scalar, y / scalar, z / scalar);
	}

	constexpr Vector3f operator-() const
	{
	    return Vector3f(-x, -y, -z);
	}

	constexpr Vector3f& operator+=(const Vector3f& rhs)
	{
	    x += rhs.x;
	    y += rhs.y;
	    z += rhs.z;
	    return *this;
	}

	constexpr Vector3f& operator-=(const Vector3f& rhs)
	{
	    x -= rhs.x;
	    y -= rhs.y;
	    z -= rhs.z;
	    return *this;
	}

	constexpr Vector3f& operator+=(const float& scalar)
	{
	    x += scalar;
	    y += scalar;
	    z += scalar;
	    return *this;
	}

	constexpr Vector3f& operator-=(const float& scalar)
	{
	    x -= scalar;
	    y -= scalar;
	    z -= scalar;
	    return *this;
	}

	constexpr Vector3f& operator*=(const Vector3f& rhs)
	{
	    x *= rhs.x;
	    y *= rhs.y;
	    z *= rhs.z;
	    return *this;
	}

	constexpr Vector3f& operator*=(const float& scalar)
	{
	    x *= scalar;
	    y *= scalar;
	    z *= scalar;
	    return *this;
	}

	constexpr Vector3f& operator/=(const float& scalar)
	{
	    x /= scalar;
	    y /= scalar;
	    z /= scalar;
	    return *this;
	}

	constexpr Vector3 vec3() const
	{
		return Vector3{x, y, z};
	}

	constexpr float dot(const Vector3f& rhs) const
	{
	    return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	constexpr Vector3f cross(const Vector3f& rhs) const
	{
	    return Vector3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
	}

	constexpr float length() const
	{
	    return std::sqrt(x * x + y * y + z * z);
	}

	constexpr float lengthSqr() const
	{
	    return x * x + y * y + z * z;
	}

	constexpr float distance(const Vector3f& other) const
	{
	    float dx = other.x - x;
	    float dy = other.y - y;
	    float dz = other.z - z;
	    return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	constexpr Vector3f& normalize()
	{
	    float len = length();
	    if (len > 0)
	    {
	        x /= len;
	        y /= len;
	        z /= len;
	    }
	    return *this;
	}
};

constexpr Vector3f Vector3fZero()
{
    return Vector3f {0, 0, 0};
}