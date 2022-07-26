#pragma once

#include <cmath>

#include "../Helpers.h"
#include "Utils.h"
#include <stdlib.h>

#pragma once
#ifndef RAD2DEG
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI_F) )
#endif

#ifndef DEG2RAD
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#endif
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.

#define M_PHI		1.61803398874989484820 // golden ratio
#define CHECK_VALID( _v ) 0
#define Assert( _exp ) ((void)0)
#define FastSqrt(x)			(sqrt)(x)

class matrix3x4;
struct matrix3x4_t
{
    matrix3x4_t() {}
    matrix3x4_t(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23)
    {
        m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
        m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
        m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
    }

    float* operator[](int i) { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
    const float* operator[](int i) const { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
    float* Base() { return &m_flMatVal[0][0]; }
    const float* Base() const { return &m_flMatVal[0][0]; }
    Vector GetOrigin() {
        return Vector(m_flMatVal[0][3], m_flMatVal[1][3], m_flMatVal[2][3]);
    }

    float m_flMatVal[3][4];
};
struct Vector {
    Vector() = default;
    constexpr Vector(float x, float y, float z) noexcept : x{ x }, y{ y }, z{ z } {}

    constexpr auto notNull() const noexcept
    {
        return x || y || z;
    }
    
    friend constexpr auto operator==(const Vector& a, const Vector& b) noexcept
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }

    friend constexpr auto operator!=(const Vector& a, const Vector& b) noexcept
    {
        return !(a == b);
    }

    constexpr Vector& operator=(const float array[3]) noexcept
    {
        x = array[0];
        y = array[1];
        z = array[2];
        return *this;
    }
    constexpr Vector& operator=(const Vector& a) noexcept
    {
        x = a.x;
        y = a.y;
        z = a.z;
        return *this;
    }
    constexpr Vector& operator+=(const Vector& v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    constexpr Vector& operator+=(float f) noexcept
    {
        x += f;
        y += f;
        z += f;
        return *this;
    }

    constexpr Vector& operator-=(const Vector& v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    constexpr Vector& operator-=(float f) noexcept
    {
        x -= f;
        y -= f;
        z -= f;
        return *this;
    }

    friend constexpr auto operator-(const Vector& a, const Vector& b) noexcept
    {
        return Vector{ a.x - b.x, a.y - b.y, a.z - b.z };
    }

    friend constexpr auto operator+(const Vector& a, const Vector& b) noexcept
    {
        return Vector{ a.x + b.x, a.y + b.y, a.z + b.z };
    }
    
    friend constexpr auto operator*(const Vector& a, const Vector& b) noexcept
    {
        return Vector{ a.x * b.x, a.y * b.y, a.z * b.z };
    }

    constexpr Vector& operator/=(float div) noexcept
    {
        x /= div;
        y /= div;
        z /= div;
        return *this;
    }
    constexpr Vector& operator*=(float div) noexcept
    {
        x *= div;
        y *= div;
        z *= div;
        return *this;
    }
    constexpr auto operator*(float mul) const noexcept
    {
        return Vector{ x * mul, y * mul, z * mul };
    }
    constexpr auto operator/(float mul) const noexcept
    {
        return Vector{ x / mul, y / mul, z / mul };
    }
    constexpr auto operator-(float sub) const noexcept
    {
        return Vector{ x - sub, y - sub, z - sub };
    }

    constexpr auto operator+(float add) const noexcept
    {
        return Vector{ x + add, y + add, z + add };
    }
    float Dot(const Vector& vOther) const noexcept
    {
        const Vector& a = *this;

        return(a.x * vOther.x + a.y * vOther.y + a.z * vOther.z);
    }
    float Length(void) const noexcept
    {

        return sqrt(x * x + y * y + z * z);
    }
    Vector& normalize() noexcept
    {
        x = std::isfinite(x) ? std::remainder(x, 360.0f) : 0.0f;
        y = std::isfinite(y) ? std::remainder(y, 360.0f) : 0.0f;
        z = 0.0f;
        return *this;
    }
   Vector Normalize()
    {
        Vector vector;
        float length = this->Length();

        if (length != 0)
        {
            vector.x = x / length;
            vector.y = y / length;
            vector.z = z / length;
        }
        else
        {
            vector.x = vector.y = 0.0f; vector.z = 1.0f;
        }

        return vector;
    }
    //===============================================
    void NormalizeInPlace()
    {
        Vector& v = *this;

        float iradius = 1.f / (this->Length() + 1.192092896e-07F); //FLT_EPSILON

        v.x *= iradius;
        v.y *= iradius;
        v.z *= iradius;
    }
    auto length() const noexcept
    {
        return std::sqrt(x * x + y * y + z * z);
    }
    auto LengthSqr() const noexcept
    {
        return (x * x + y * y + z * z);
    }

    auto length2D() const noexcept
    {
        return std::sqrt(x * x + y * y);
    }

    constexpr auto squareLength() const noexcept
    {
        return x * x + y * y + z * z;
    }

    constexpr auto dotProduct(const Vector& v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z;
    }

    constexpr auto transform(const matrix3x4& mat) const noexcept;

    auto distTo(const Vector& v) const noexcept
    {
        return (*this - v).length();
    }

    auto toAngle() const noexcept
    {
        return Vector{ Helpers::rad2deg(std::atan2(-z, std::hypot(x, y))),
                       Helpers::rad2deg(std::atan2(y, x)),
                       0.0f };
    }

    static auto fromAngle(const Vector& angle) noexcept
    {
        return Vector{ std::cos(Helpers::deg2rad(angle.x)) * std::cos(Helpers::deg2rad(angle.y)),
                       std::cos(Helpers::deg2rad(angle.x)) * std::sin(Helpers::deg2rad(angle.y)),
                      -std::sin(Helpers::deg2rad(angle.x)) };
    }

    float x, y, z;
};

#include "matrix3x4.h"

class QAngleByValue;

class QAngle
{
public:
    // Members
    float x, y, z;

    // Construction/destruction
    QAngle(void);
    QAngle(float X, float Y, float Z);
    //      QAngle(RadianEuler const &angles);      // evil auto type promotion!!!

    // Allow pass-by-value
    operator QAngleByValue& () { return *((QAngleByValue*)(this)); }
    operator const QAngleByValue& () const { return *((const QAngleByValue*)(this)); }

    // Initialization
    void Init(float ix = 0.0f, float iy = 0.0f, float iz = 0.0f);
    void Random(float minVal, float maxVal);

    // Got any nasty NAN's?
    bool IsValid() const;
    void Invalidate();

    // array access...
    float operator[](int i) const;
    float& operator[](int i);

    // Base address...
    float* Base();
    float const* Base() const;

    // equality
    bool operator==(const QAngle& v) const;
    bool operator!=(const QAngle& v) const;

    bool IsZero(float tolerance = 0.01f) const
    {
        return (x > -tolerance && x < tolerance&&
            y > -tolerance && y < tolerance&&
            z > -tolerance && z < tolerance);
    }

    constexpr auto notNull() const noexcept
    {
        return x || y || z;
    }

    // arithmetic operations
    QAngle& operator+=(const QAngle& v);
    QAngle& operator-=(const QAngle& v);
    QAngle& operator*=(float s);
    QAngle& operator/=(float s);

    // Get the vector's magnitude.
    float   Length() const;
    float   LengthSqr() const;

    // negate the QAngle components
    //void  Negate();

    // No assignment operators either...
    QAngle& operator=(const QAngle& src);

#ifndef VECTOR_NO_SLOW_OPERATIONS
    // copy constructors

    // arithmetic operations
    QAngle  operator-(void) const;

    QAngle  operator+(const QAngle& v) const;
    QAngle  operator-(const QAngle& v) const;
    QAngle  operator*(float fl) const;
    QAngle  operator/(float fl) const;
#else

private:
    // No copy constructors allowed if we're in optimal mode
    QAngle(const QAngle& vOther);

#endif
};

//-----------------------------------------------------------------------------
// constructors
//-----------------------------------------------------------------------------
inline QAngle::QAngle(void) noexcept
{
#ifdef _DEBUG
#ifdef VECTOR_PARANOIA
    // Initialize to NAN to catch errors
    x = y = z = VEC_T_NAN;
#endif
#endif
}

inline QAngle::QAngle(float X, float Y, float Z) noexcept
{
    x = X; y = Y; z = Z;
    CHECK_VALID(*this);
}

//-----------------------------------------------------------------------------
// initialization
//-----------------------------------------------------------------------------
inline void QAngle::Init(float ix, float iy, float iz) noexcept
{
    x = ix; y = iy; z = iz;
    CHECK_VALID(*this);
}

inline void QAngle::Random(float minVal, float maxVal) noexcept
{
    x = minVal + ((float)rand() / (float)RAND_MAX) * (maxVal - minVal);
    y = minVal + ((float)rand() / (float)RAND_MAX) * (maxVal - minVal);
    z = minVal + ((float)rand() / (float)RAND_MAX) * (maxVal - minVal);
    CHECK_VALID(*this);
}

//-----------------------------------------------------------------------------
// assignment
//-----------------------------------------------------------------------------
inline QAngle& QAngle::operator=(const QAngle& vOther) noexcept
{
    CHECK_VALID(vOther);
    x = vOther.x; y = vOther.y; z = vOther.z;
    return *this;
}

//-----------------------------------------------------------------------------
// comparison
//-----------------------------------------------------------------------------
inline bool QAngle::operator==(const QAngle& src) const noexcept
{
    CHECK_VALID(src);
    CHECK_VALID(*this);
    return (src.x == x) && (src.y == y) && (src.z == z);
}

inline bool QAngle::operator!=(const QAngle& src) const noexcept
{
    CHECK_VALID(src);
    CHECK_VALID(*this);
    return (src.x != x) || (src.y != y) || (src.z != z);
}

//-----------------------------------------------------------------------------
// standard math operations
//-----------------------------------------------------------------------------
inline QAngle& QAngle::operator+=(const QAngle& v) noexcept
{
    CHECK_VALID(*this);
    CHECK_VALID(v);
    x += v.x; y += v.y; z += v.z;
    return *this;
}

inline QAngle& QAngle::operator-=(const QAngle& v) noexcept
{
    CHECK_VALID(*this);
    CHECK_VALID(v);
    x -= v.x; y -= v.y; z -= v.z;
    return *this;
}

inline QAngle& QAngle::operator*=(float fl) noexcept
{
    x *= fl;
    y *= fl;
    z *= fl;
    CHECK_VALID(*this);
    return *this;
}

inline QAngle& QAngle::operator/=(float fl) noexcept
{
    Assert(fl != 0.0f);
    float oofl = 1.0f / fl;
    x *= oofl;
    y *= oofl;
    z *= oofl;
    CHECK_VALID(*this);
    return *this;
}

//-----------------------------------------------------------------------------
// Base address...
//-----------------------------------------------------------------------------
inline float* QAngle::Base() noexcept
{
    return (float*)this;
}

inline float const* QAngle::Base() const noexcept
{
    return (float const*)this;
}

//-----------------------------------------------------------------------------
// Array access
//-----------------------------------------------------------------------------
inline float& QAngle::operator[](int i) noexcept
{
    Assert((i >= 0) && (i < 3));
    return ((float*)this)[i];
}

inline float QAngle::operator[](int i) const noexcept
{
    Assert((i >= 0) && (i < 3));
    return ((float*)this)[i];
}

//-----------------------------------------------------------------------------
// length
//-----------------------------------------------------------------------------
inline float QAngle::Length() const noexcept
{

    return (float)FastSqrt(LengthSqr());
}


inline float QAngle::LengthSqr() const noexcept
{

    return x * x + y * y + z * z;
}


//-----------------------------------------------------------------------------
// arithmetic operations (SLOW!!)
//-----------------------------------------------------------------------------
#ifndef VECTOR_NO_SLOW_OPERATIONS

inline QAngle QAngle::operator-(void) const noexcept
{
    return QAngle(-x, -y, -z);
}

inline QAngle QAngle::operator+(const QAngle& v) const noexcept
{
    QAngle res;
    res.x = x + v.x;
    res.y = y + v.y;
    res.z = z + v.z;
    return res;
}

inline QAngle QAngle::operator-(const QAngle& v) const noexcept
{
    QAngle res;
    res.x = x - v.x;
    res.y = y - v.y;
    res.z = z - v.z;
    return res;
}

inline QAngle QAngle::operator*(float fl) const noexcept
{
    QAngle res;
    res.x = x * fl;
    res.y = y * fl;
    res.z = z * fl;
    return res;
}

inline QAngle QAngle::operator/(float fl) const noexcept
{
    QAngle res;
    res.x = x / fl;
    res.y = y / fl;
    res.z = z / fl;
    return res;
}

inline QAngle operator*(float fl, const QAngle& v) noexcept
{
    return v * fl;
}

#endif // VECTOR_NO_SLOW_OPERATIONS


//QANGLE SUBTRAC
inline void QAngleSubtract(const QAngle& a, const QAngle& b, QAngle& c) noexcept
{

    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
}

//QANGLEADD
inline void QAngleAdd(const QAngle& a, const QAngle& b, QAngle& c) noexcept
{

    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
}

constexpr auto Vector::transform(const matrix3x4& mat) const noexcept
{
    return Vector{ dotProduct({ mat[0][0], mat[0][1], mat[0][2] }) + mat[0][3],
                   dotProduct({ mat[1][0], mat[1][1], mat[1][2] }) + mat[1][3],
                   dotProduct({ mat[2][0], mat[2][1], mat[2][2] }) + mat[2][3] };
}

