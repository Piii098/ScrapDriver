#pragma once
#include "DirectX/DirectX.h"

class RootSignature
{
public:
	RootSignature(); // コンストラクタでルートシグネチャを生成
	bool Initialize(); // ルートシグネチャの生成
	bool IsValid(); // ルートシグネチャの生成に成功したかどうかを返す
	ID3D12RootSignature* Get(); // ルートシグネチャを返す

private:
	bool _isValid = false; // ルートシグネチャの生成に成功したかどうか
	ComPtr<ID3D12RootSignature> _rootSignature = nullptr; // ルートシグネチャ
};