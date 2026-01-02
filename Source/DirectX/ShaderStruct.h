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

// シーン共通定数（b0）: View / Projection のみ
struct Transform
{
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

// レイトレーシング用ライトデータ（シーン共通）
// 係数（拡散/鏡面/反射の強さなど）は PerMeshMaterialData 側で制御する。
struct RaytracingLightingData {
	Vector3 lightPosition;      // ライト位置
	float ambientIntensity;     // 環境光の強さ（0-1）
	Vector4 lightColor;         // ライト色（RGBA）
};

// メッシュごとのマテリアルデータ
struct PerMeshMaterialData {
	Vector4 baseColor;          // 基本色（RGBA）
	float metallic;             // メタリック（0.0-1.0）
	float roughness;            // ラフネス（0.0-1.0）
	float reflectance;          // 反射率（0.0-1.0）
	unsigned int materialType;  // マテリアルタイプ（0=Diffuse, 1=Metal, 2=Glass）
	Vector2 uvScale;            // UVスケール
	Vector2 uvOffset;           // UVオフセット
	Vector3 uvAxis1;            // UV計算用の軸1（例: X軸）
	float uvRange1;             // 軸1の範囲
	Vector3 uvAxis2;            // UV計算用の軸2（例: Z軸）
	float uvRange2;             // 軸2の範囲
	unsigned int textureIndex;  // テクスチャディスクリプタのインデックス
	float padding[3];           // 16バイトアラインメント用パディング
};

// ライトデータ（シェーダーで使用）
struct LightData {
	Vector3 position;           // ライトの位置
	unsigned int type;          // ライトタイプ（0=Directional, 1=Point, 2=Spot）
	Vector3 direction;          // ライトの方向（Directional/Spot用）
	float range;                // 範囲（Point/Spot用）
	Vector4 color;              // ライトの色（RGBA）
	float intensity;            // 強度
	float innerAngle;           // 内側の角度（Spot用、ラジアン）
	float outerAngle;           // 外側の角度（Spot用、ラジアン）
	float _padding;
};

// ライト配列データ
struct LightsData {
	unsigned int lightCount;    // ライト数
	float _padding[3];
	LightData lights[8];        // 最大8ライト
};

// シャドウマップ用のライト定数バッファ（シェーダー側と一致させる）
struct ShadowLightsData {
	unsigned int lightCount;
	float _padding0[3];
	LightData lights[8];
	Matrix4x4 lightViewProj;    // ライト空間のビュー・プロジェクション行列
};