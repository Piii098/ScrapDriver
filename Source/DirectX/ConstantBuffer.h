#pragma once
#include "DirectX.h"
#include "DirectX/ShaderStruct.h"

class ConstantBuffer
{
public:
    ConstantBuffer(); 
    bool IsValid(); // バッファ生成に成功したかを返す

	ID3D12Resource* GetResource() const { return _constantBuffer.Get(); } // 定数バッファリソースを返す
    Transform* GetPtr() const; // 定数バッファにマッピングされたポインタを返す

private:
    bool _isValid = false; // 定数バッファ生成に成功したか
    ComPtr<ID3D12Resource> _constantBuffer; // 定数バッファ
    Transform* _mappedPtr = nullptr;

};
