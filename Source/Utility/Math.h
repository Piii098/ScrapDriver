#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <DirectXMath.h>

namespace Math {

	template <typename T>
	T Max(const T& a, const T& b)
	{
		return glm::max(a, b);
	}

	template <typename T>
	T Min(const T& a, const T& b)
	{
		return glm::min(a, b);
	}

	template <typename T>
	T Clamp(const T& value, const T& lower, const T& upper)
	{
		return glm::clamp(value, lower, upper);
	}

	template<typename T>
	inline int Sign(T value) {
		return glm::sign(value);
	}
	
	inline float Abs(float value) {
		return glm::abs(value);
	}

	inline float Cos(float angle)
	{
		return glm::cos(angle);
	}

	inline float Sin(float angle)
	{
		return glm::sin(angle);
	}

	inline float Tan(float angle)
	{
		return glm::tan(angle);
	}

	inline float Acos(float value)
	{
		return glm::acos(value);
	}

	inline float Atan2(float y, float x)
	{
		return glm::atan(y, x);
	}

	inline float Cot(float angle)
	{
		return 1.0f / glm::tan(angle);
	}

	inline float Lerp(float a, float b, float f)
	{
		return glm::mix(a, b, f);
	}

	inline float Sqrt(float value)
	{
		return glm::sqrt(value);
	}

	template<typename T>
	inline T Length(const T& vec)
	{
		return glm::length(vec);
	}

	template<typename T>
	inline T LengthSq(const T& vec)
	{
		return glm::dot(vec, vec);
	}

	template<typename T>
	inline T Normalize(const T& vec)
	{
		return glm::normalize(vec);
	}

	template<typename T>
	inline float Dot(const T& a, const T& b)
	{
		return glm::dot(a, b);
	}

	template<typename T>
	inline T Cross(const T& a, const T& b)
	{
		return glm::cross(a, b);
	}

	template<typename T>
	inline T Reflect(const T& incident, const T& normal)
	{
		return glm::reflect(incident, normal);
	}

	template<typename T, typename U>
	inline T Lerp(const T& a, const T& b, U f)
	{
		return glm::mix(a, b, f);
	}

	template<typename T>
	inline float Distance(const T& a, const T& b)
	{
		return glm::distance(a, b);
	}

};
using namespace DirectX;

struct Vector2 : public XMFLOAT2 {
	Vector2() = default;
	Vector2(float x, float y)
	{
		this->x = x; this->y = y;
	}
	//コンストラクタ
	Vector2(const XMVECTOR& other) :XMFLOAT2() {
		XMVECTOR temp = other;
		XMStoreFloat2(this, temp);
	}
	inline bool operator == (const Vector2& r) const { return x == r.x && y == r.y; }
	inline bool operator != (const Vector2& r) const { return x != r.x || y != r.y; }
	inline XMVECTOR operator *=(const float r) { x *= r; y *= r; return *this; }
	inline XMVECTOR operator *(const float r) const { return Vector2(x * r, y * r); }
	inline XMVECTOR operator /(const float r) const { return Vector2(x / r, y / r); }
	// ベクトルの内積
	float Dot(Vector2 In) { return x * In.x + y * In.y; }
	// ベクトルの外積
	float Cross(Vector2 In) { return x * In.y - y * In.x; }
	// ベクトルのスケーリング
	Vector2 Scale(float Scale) { Vector2 Result = { x * Scale, y * Scale }; return Result; }
	//代入
	Vector2& operator=(const XMVECTOR& other) {
		XMVECTOR temp = other;
		XMStoreFloat2(this, temp);
		return *this;
	}
	//キャスト
	operator XMVECTOR() const {
		return XMLoadFloat2(this);
	}
	//長さ
	float Length() const {
		return (static_cast<Vector2>(XMVector2Length(XMVECTOR(*this)))).x;
	}
	//正規化
	void Normalize() {
		*this = XMVector2Normalize(XMVECTOR(*this));
	}
	static const Vector2 Zero;
	static const Vector2 One;
	static const Vector2 Up;
	static const Vector2 Down;
	static const Vector2 Right;
	static const Vector2 Left;
};

struct Vector3 : public XMFLOAT3 {
	Vector3() = default;
	Vector3(float x, float y, float z)
	{
		this->x = x; this->y = y; this->z = z;
	}
	//コンストラクタ
	Vector3(const XMVECTOR& other) :XMFLOAT3() {
		XMVECTOR temp = other;
		XMStoreFloat3(this, temp);
	}

	inline bool operator == (const Vector3& r) const { return x == r.x && y == r.y && z == r.z; }
	inline bool operator != (const Vector3& r) const { return x != r.x || y != r.y || z != r.z; }
	inline XMVECTOR operator *(const float r) const { return Vector3(x * r, y * r, z * r); }
	inline XMVECTOR operator /(const float r) const { return Vector3(x / r, y / r, z / r); }

	// ベクトルの内積
	float Dot(Vector3 In) { return x * In.x + y * In.y + z * In.z; }
	// ベクトルの外積
	Vector3 Cross(Vector3 In) { return Vector3(y * In.z - z * In.y, z * In.x - x * In.z, x * In.y - y * In.x); }
	// ベクトルのスケーリング
	Vector3 Scale(float Scale) { Vector3 Result = { x * Scale, y * Scale, z * Scale }; return Result; }

	//代入
	Vector3& operator=(const XMVECTOR& other) {
		XMVECTOR temp = other;
		XMStoreFloat3(this, temp);
		return *this;
	}
	//キャスト
	operator XMVECTOR() const {
		return XMLoadFloat3(this);
	}
	//長さ
	float Length() const {
		return (static_cast<Vector3>(XMVector3Length(XMVECTOR(*this)))).x;
	}
	//正規化
	void Normalize() {
		*this = XMVector3Normalize(XMVECTOR(*this));
	}

	static const Vector3 Zero;
	static const Vector3 One;
	static const Vector3 Up;
	static const Vector3 Down;
	static const Vector3 Right;
	static const Vector3 Left;
	static const Vector3 Forward;
	static const Vector3 Backward;
};

struct Vector4 : public XMFLOAT4
{
	Vector4() = default;
	Vector4(float x, float y, float z, float w)
	{
		this->x = x; this->y = y; this->z = z; this->w = w;
	}
	//コンストラクタ
	Vector4(const XMVECTOR& other) :XMFLOAT4() {
		XMVECTOR temp = other;
		XMStoreFloat4(this, temp);
	}
	//代入
	Vector4& operator=(const XMVECTOR& other) {
		XMVECTOR temp = other;
		XMStoreFloat4(this, temp);
		return *this;
	}
	//キャスト
	operator XMVECTOR() const {
		return XMLoadFloat4(this);
	}

	static const Vector4 UnitX;
	static const Vector4 UnitY;
	static const Vector4 UnitZ;
	static const Vector4 UnitW;
	static const Vector4 Zero;
	static const Vector4 One;
	static const Vector4 Up;
	static const Vector4 Down;
	static const Vector4 Right;
	static const Vector4 Left;
	static const Vector4 Forward;
	static const Vector4 Backward;
};


struct Matrix4x4 : public XMMATRIX
{
	Matrix4x4()
	{
		*this = XMMatrixIdentity();
	}
	Matrix4x4(const XMMATRIX& other) :XMMATRIX(other) {}
	Matrix4x4(const Matrix4x4& other) :XMMATRIX(other) {}
	
	// コピー代入演算子
	Matrix4x4& operator=(const XMMATRIX& other) {
		XMMATRIX::operator=(other);
		return *this;
	}
	Matrix4x4& operator=(const Matrix4x4& other) {
		XMMATRIX::operator=(other);
		return *this;
	}
	
	//XMFLOAT4X4への変換代入
	XMFLOAT4X4& operator=(const XMFLOAT4X4& other) {
		*this = XMLoadFloat4x4(&other);
		return *reinterpret_cast<XMFLOAT4X4*>(this);
	}
	
	//XMFLOAT4X4へのキャスト
	operator XMFLOAT4X4() const {
		XMFLOAT4X4 f4_4;
		XMStoreFloat4x4(&f4_4, *this);
		return f4_4;
	}

	void Transpose() {
		*this = XMMatrixTranspose(*this);
	}

	void Set(int p, Vector3 m)
	{
		r[p] = XMVectorSet(m.x, m.y, m.z, 1.0f);
	}
	void Set(const XMFLOAT4X4* other) {
		*this = XMLoadFloat4x4(other);
	}

	static const Matrix4x4 Identity;
	static const Matrix4x4 Zero;
	
};