#pragma once
#include "Framework.h"
#include "DirectX.h"
#include "../Utility/Math.h"

struct Vertex {
	Vector3 Position; // ・ｽﾊ置・ｽ・ｽ・ｽW
	Vector3 Normal; // ・ｽ@・ｽ・ｽ
	Vector2 UV; // uv・ｽ・ｽ・ｽW
	Vector3 Tangent; // ・ｽﾚ具ｿｽ・ｽ
	Vector4 Color; // ・ｽ・ｽ・ｽ_・ｽF

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 5;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) Transform
{
	Matrix4x4 World; // ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽ
	Matrix4x4 View; // ・ｽr・ｽ・ｽ・ｽ[・ｽs・ｽ・ｽ
	Matrix4x4 Proj; // ・ｽ・ｽ・ｽe・ｽs・ｽ・ｽ
};

struct Mesh
{
	std::vector<Vertex> Vertices; // ・ｽ・ｽ・ｽ_・ｽf・ｽ[・ｽ^・ｽﾌ配・ｽ・ｽ
	std::vector<uint32_t> Indices; // ・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽﾌ配・ｽ・ｽ
	std::wstring DiffuseMap; // ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽp・ｽX
};