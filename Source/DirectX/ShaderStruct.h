#pragma once
#include "Framework.h"
#include "DirectX.h"
#include "Utility/Math.h"

struct Vertex {
	Vector3 Position;
	Vector3 Normal;
	Vector2 UV;
	Vector3 Tangent;
	Vector4 Color;

	static std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
};

struct Transform
{
	Matrix4x4 World; // 
	Matrix4x4 View; // 
	Matrix4x4 Proj; // 
};

struct Mesh
{
	std::vector<Vertex> Vertices; // 
	std::vector<unsigned short> Indices; // 
	std::wstring DiffuseMap; // X
};

// レイトレーシング用カメラデータ
struct RaytracingCameraData {
	Vector3 Position;
	float FOV;              // 視野角（ラジアン）
	Vector3 Forward;
	float AspectRatio;      // アスペクト比
	Vector3 Right;
	float _padding1;
	Vector3 Up;
	float _padding2;
};

// レイトレーシング用マテリアルデータ
struct RaytracingMaterialData {
	float reflectanceCoef;      // 反射係数
	float diffuseCoef;          // 拡散反射係数
	float specularCoef;         // 鏡面反射係数
	float specularPower;        // 鏡面反射の鋭さ
	Vector3 lightPosition;      // ライト位置
	float _padding3;
	Vector4 lightColor;         // ライト色（RGBA）
};