#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

class Vector2
{
public:
	union {
		struct { float x, y; };
		struct { float s, t; };
		struct { float r, g; };
	};
	Vector2() : x(0.0f), y(0.0f) {}
	~Vector2() {}
	Vector2(float num) : x(num), y(num) {}
	Vector2(float x, float y) : x(x), y(y) {}
	Vector2(const Vector2& u) : x(u.x), y(u.y) {}
	Vector2& operator = (const Vector2& u) { x = u.x; y = u.y; return *this; }
	Vector2 operator - () { return Vector2(-x, -y); }
	float* operator & () { return (float*)this; };
	Vector2& operator += (float num) { x += num; y += num; return *this; }
	Vector2& operator += (const Vector2& u) { x += u.x; y += u.y; return *this; }
	Vector2& operator -= (float num) { x -= num; y -= num; return *this; }
	Vector2& operator -= (const Vector2& u) { x -= u.x; y -= u.y; return *this; }
	Vector2& operator *= (float num) { x *= num; y *= num; return *this; }
	Vector2& operator *= (const Vector2& u) { x *= u.x; y *= u.y; return *this; }
	Vector2& operator /= (float num) { x /= num; y /= num; return *this; }
	Vector2& operator /= (const Vector2& u) { x /= u.x; y /= u.y; return *this; }
	friend Vector2 operator + (const Vector2& u, float num) { return Vector2(u.x + num, u.y + num); }
	friend Vector2 operator + (float num, const Vector2& u) { return Vector2(num + u.x, num + u.y); }
	friend Vector2 operator + (const Vector2& u, const Vector2& v) { return Vector2(u.x + v.x, u.y + v.y); }
	friend Vector2 operator - (const Vector2& u, float num) { return Vector2(u.x - num, u.y - num); }
	friend Vector2 operator - (float num, const Vector2& u) { return Vector2(num - u.x, num - u.y); }
	friend Vector2 operator - (const Vector2& u, const Vector2& v) { return Vector2(u.x - v.x, u.y - v.y); }
	friend Vector2 operator * (const Vector2& u, float num) { return Vector2(u.x * num, u.y * num); }
	friend Vector2 operator * (float num, const Vector2& u) { return Vector2(num * u.x, num * u.y); }
	friend Vector2 operator * (const Vector2& u, const Vector2& v) { return Vector2(u.x * v.x, u.y * v.y); }
	friend Vector2 operator / (const Vector2& u, float num) { return Vector2(u.x / num, u.y / num); }
	friend Vector2 operator / (float num, const Vector2& u) { return Vector2(num / u.x, num / u.y); }
	friend Vector2 operator / (const Vector2& u, const Vector2& v) { return Vector2(u.x / v.x, u.y / v.y); }
};

class Vector3
{
public:
	union {
		struct { float x, y, z; };
		struct { float s, t, p; };
		struct { float r, g, b; };
	};
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	~Vector3() {}
	Vector3(float num) : x(num), y(num), z(num) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector3(const Vector2& u, float z) : x(u.x), y(u.y), z(z) {}
	Vector3(const Vector3& u) : x(u.x), y(u.y), z(u.z) {}
	Vector3& operator = (const Vector3& u) { x = u.x; y = u.y; z = u.z; return *this; }
	Vector3 operator - () { return Vector3(-x, -y, -z); }
	float* operator & () { return (float*)this; }
	operator Vector2 () { return *(Vector2*)this; }
	Vector3& operator += (float num) { x += num; y += num; z += num; return *this; }
	Vector3& operator += (const Vector3& u) { x += u.x; y += u.y; z += u.z; return *this; }
	Vector3& operator -= (float num) { x -= num; y -= num; z -= num; return *this; }
	Vector3& operator -= (const Vector3& u) { x -= u.x; y -= u.y; z -= u.z; return *this; }
	Vector3& operator *= (float num) { x *= num; y *= num; z *= num; return *this; }
	Vector3& operator *= (const Vector3& u) { x *= u.x; y *= u.y; z *= u.z; return *this; }
	Vector3& operator /= (float num) { x /= num; y /= num; z /= num; return *this; }
	Vector3& operator /= (const Vector3& u) { x /= u.x; y /= u.y; z /= u.z; return *this; }
	friend Vector3 operator + (const Vector3& u, float num) { return Vector3(u.x + num, u.y + num, u.z + num); }
	friend Vector3 operator + (float num, const Vector3& u) { return Vector3(num + u.x, num + u.y, num + u.z); }
	friend Vector3 operator + (const Vector3& u, const Vector3& v) { return Vector3(u.x + v.x, u.y + v.y, u.z + v.z); }
	friend Vector3 operator - (const Vector3& u, float num) { return Vector3(u.x - num, u.y - num, u.z - num); }
	friend Vector3 operator - (float num, const Vector3& u) { return Vector3(num - u.x, num - u.y, num - u.z); }
	friend Vector3 operator - (const Vector3& u, const Vector3& v) { return Vector3(u.x - v.x, u.y - v.y, u.z - v.z); }
	friend Vector3 operator * (const Vector3& u, float num) { return Vector3(u.x * num, u.y * num, u.z * num); }
	friend Vector3 operator * (float num, const Vector3& u) { return Vector3(num * u.x, num * u.y, num * u.z); }
	friend Vector3 operator * (const Vector3& u, const Vector3& v) { return Vector3(u.x * v.x, u.y * v.y, u.z * v.z); }
	friend Vector3 operator / (const Vector3& u, float num) { return Vector3(u.x / num, u.y / num, u.z / num); }
	friend Vector3 operator / (float num, const Vector3& u) { return Vector3(num / u.x, num / u.y, num / u.z); }
	friend Vector3 operator / (const Vector3& u, const Vector3& v) { return Vector3(u.x / v.x, u.y / v.y, u.z / v.z); }
};

class Vector4
{
public:
	union {
		struct { float x, y, z, w; };
		struct { float s, t, p, q; };
		struct { float r, g, b, a; };
	};
	Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	~Vector4() {}
	Vector4(float num) : x(num), y(num), z(num), w(num) {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Vector4(const Vector2& u, float z, float w) : x(u.x), y(u.y), z(z), w(w) {}
	Vector4(const Vector3& u, float w) : x(u.x), y(u.y), z(u.z), w(w) {}
	Vector4(const Vector4& u) : x(u.x), y(u.y), z(u.z), w(u.w) {}
	Vector4& operator = (const Vector4& u) { x = u.x; y = u.y; z = u.z; w = u.w; return *this; }
	Vector4 operator - () { return Vector4(-x, -y, -z, -w); }
	float* operator & () { return (float*)this; }
	operator Vector2 () { return *(Vector2*)this; }
	operator Vector3 () { return *(Vector3*)this; }
	Vector4& operator += (float num) { x += num; y += num; z += num; w += num; return *this; }
	Vector4& operator += (const Vector4& u) { x += u.x; y += u.y; z += u.z; w += u.w; return *this; }
	Vector4& operator -= (float num) { x -= num; y -= num; z -= num; w -= num; return *this; }
	Vector4& operator -= (const Vector4& u) { x -= u.x; y -= u.y; z -= u.z; w -= u.w; return *this; }
	Vector4& operator *= (float num) { x *= num; y *= num; z *= num; w *= num; return *this; }
	Vector4& operator *= (const Vector4& u) { x *= u.x; y *= u.y; z *= u.z; w *= u.w; return *this; }
	Vector4& operator /= (float num) { x /= num; y /= num; z /= num; w /= num; return *this; }
	Vector4& operator /= (const Vector4& u) { x /= u.x; y /= u.y; z /= u.z; w /= u.w; return *this; }
	friend Vector4 operator + (const Vector4& u, float num) { return Vector4(u.x + num, u.y + num, u.z + num, u.w + num); }
	friend Vector4 operator + (float num, const Vector4& u) { return Vector4(num + u.x, num + u.y, num + u.z, num + u.w); }
	friend Vector4 operator + (const Vector4& u, const Vector4& v) { return Vector4(u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w); }
	friend Vector4 operator - (const Vector4& u, float num) { return Vector4(u.x - num, u.y - num, u.z - num, u.w - num); }
	friend Vector4 operator - (float num, const Vector4& u) { return Vector4(num - u.x, num - u.y, num - u.z, num - u.w); }
	friend Vector4 operator - (const Vector4& u, const Vector4& v) { return Vector4(u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w); }
	friend Vector4 operator * (const Vector4& u, float num) { return Vector4(u.x * num, u.y * num, u.z * num, u.w * num); }
	friend Vector4 operator * (float num, const Vector4& u) { return Vector4(num * u.x, num * u.y, num * u.z, num * u.w); }
	friend Vector4 operator * (const Vector4& u, const Vector4& v) { return Vector4(u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w); }
	friend Vector4 operator / (const Vector4& u, float num) { return Vector4(u.x / num, u.y / num, u.z / num, u.w / num); }
	friend Vector4 operator / (float num, const Vector4& u) { return Vector4(num / u.x, num / u.y, num / u.z, num / u.w); }
	friend Vector4 operator / (const Vector4& u, const Vector4& v) { return Vector4(u.x / v.x, u.y / v.y, u.z / v.z, u.w / v.w); }
};

class Matrix4x4
{
public:
	float M[16];
	Matrix4x4();
	~Matrix4x4();
	Matrix4x4(const Matrix4x4& Matrix);
	Matrix4x4& operator = (const Matrix4x4& Matrix);
	float& operator [] (int Index);
	float* operator & ();
	friend Matrix4x4 operator * (const Matrix4x4& Matrix1, const Matrix4x4& Matrix2);
	friend Vector2 operator * (const Matrix4x4& Matrix, const Vector2& Vector);
	friend Vector3 operator * (const Matrix4x4& Matrix, const Vector3& Vector);
	friend Vector4 operator * (const Matrix4x4& Matrix, const Vector4& Vector);
};

float dot(const Vector2& u, const Vector2& v);
float length(const Vector2& u);
float length2(const Vector2& u);
Vector2 normalize(const Vector2& u);
Vector2 reflect(const Vector2& i, const Vector2& n);
Vector2 refract(const Vector2& i, const Vector2& n, float eta);
Vector2 rotate(const Vector2& u, float angle);

Vector3 cross(const Vector3& u, const Vector3& v);
float dot(const Vector3& u, const Vector3& v);
float length(const Vector3& u);
float length2(const Vector3& u);
Vector3 mix(const Vector3& u, const Vector3& v, float a);
Vector3 normalize(const Vector3& u);
Vector3 reflect(const Vector3& i, const Vector3& n);
Vector3 refract(const Vector3& i, const Vector3& n, float eta);
Vector3 rotate(const Vector3& u, float angle, const Vector3& v);

Matrix4x4 BiasMatrix();
Matrix4x4 BiasMatrixInverse();
Matrix4x4 ViewMatrix(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& position);
Matrix4x4 ViewMatrixInverse(Matrix4x4& V);
Matrix4x4 OrthogonalProjectionMatrix(float left, float right, float bottom, float top, float n, float f);
Matrix4x4 PerspectiveProjectionMatrix(float fovy, float x, float y, float n, float f);
Matrix4x4 PerspectiveProjectionMatrixInverse(Matrix4x4& PP);
Matrix4x4 RotationMatrix(float angle, const Vector3& u);
Matrix4x4 ScaleMatrix(float x, float y, float z);
Matrix4x4 TranslationMatrix(float x, float y, float z);