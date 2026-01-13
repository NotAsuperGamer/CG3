#pragma once

#include <cmath>
#include <ostream>
#include <string>

template <class T>
struct Vec2
{
    T x{}, y{};

    constexpr Vec2() = default;

    constexpr Vec2(T x_, T y_) : x(x_), y(y_)
    {
    }

    template <class U>
    constexpr explicit Vec2(const Vec2<U>& v)
        : x(static_cast<T>(v.x)), y(static_cast<T>(v.y))
    {
    }

    constexpr T& operator[](int i) { return (i == 0) ? x : y; }
    constexpr const T& operator[](int i) const { return (i == 0) ? x : y; }

    constexpr Vec2 operator+(const Vec2& v) const { return {x + v.x, y + v.y}; }
    constexpr Vec2 operator-(const Vec2& v) const { return {x - v.x, y - v.y}; }
    constexpr Vec2 operator*(T s) const { return {x * s, y * s}; }
    constexpr Vec2 operator/(T s) const { return {x / s, y / s}; }

    Vec2& operator+=(const Vec2& v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vec2& operator-=(const Vec2& v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vec2& operator*=(T s)
    {
        x *= s;
        y *= s;
        return *this;
    }

    Vec2& operator/=(T s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    constexpr T dot(const Vec2& v) const { return x * v.x + y * v.y; }
    T length() const { return std::sqrt(dot(*this)); }

    Vec2 normalized() const
    {
        T L = length();
        return (L == T(0)) ? *this : (*this / L);
    }
};

template <class T>
inline std::ostream& operator<<(std::ostream& os, const Vec2<T>& v)
{
    return os << "(" << v.x << ", " << v.y << ")";
}

template <class T>
struct Vec3
{
    T x{}, y{}, z{};

    constexpr Vec3() = default;

    constexpr Vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_)
    {
    }

    template <class U>
    constexpr explicit Vec3(const Vec3<U>& v)
        : x(static_cast<T>(v.x)),
          y(static_cast<T>(v.y)),
          z(static_cast<T>(v.z))
    {
    }

    constexpr T& operator[](int i)
    {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    constexpr const T& operator[](int i) const
    {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    constexpr Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
    constexpr Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
    constexpr Vec3 operator*(T s) const { return {x * s, y * s, z * s}; }
    constexpr Vec3 operator/(T s) const { return {x / s, y / s, z / s}; }

    Vec3& operator+=(const Vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3& operator*=(T s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    Vec3& operator/=(T s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    constexpr T dot(const Vec3& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    constexpr Vec3 cross(const Vec3& v) const
    {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    T length() const { return std::sqrt(dot(*this)); }

    Vec3 normalized() const
    {
        T L = length();
        return (L == T(0)) ? *this : (*this / L);
    }
};

template <class T>
inline std::ostream& operator<<(std::ostream& os, const Vec3<T>& v)
{
    return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
using Vec2i = Vec2<int>;

using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;
using Vec3i = Vec3<int>;

using String = std::string;
