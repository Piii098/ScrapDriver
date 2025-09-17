#include "Utility/Math.h"

const Vector2 Vector2::Zero(0.0f, 0.0f);
const Vector2 Vector2::One(1.0f, 1.0f);
const Vector2 Vector2::Up(0.0f, 1.0f);
const Vector2 Vector2::Down(0.0f, -1.0f);
const Vector2 Vector2::Right(1.0f, 0.0f);
const Vector2 Vector2::Left(-1.0f, 0.0f);

const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);
const Vector3 Vector3::Forward(0.0f, 0.0f, 1.0f);
const Vector3 Vector3::Backward(0.0f, 0.0f, -1.0f);

const Vector4 Vector4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 Vector4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);
const Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 Vector4::Up(0.0f, 1.0f, 0.0f, 0.0f);
const Vector4 Vector4::Down(0.0f, -1.0f, 0.0f, 0.0f);
const Vector4 Vector4::Right(1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::Left(-1.0f, 0.0f, 0.0f, 0.0f);
const Vector4 Vector4::Forward(0.0f, 0.0f, 1.0f, 0.0f);
const Vector4 Vector4::Backward(0.0f, 0.0f, -1.0f, 0.0f);

const Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

const Matrix4x4 Matrix4x4::Identity = XMMatrixIdentity();
const Matrix4x4 Matrix4x4::Zero = XMMATRIX(
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
);