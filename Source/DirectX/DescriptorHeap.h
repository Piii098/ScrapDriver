#pragma once
#include "DirectX/DirectX.h"
#include <vector>

class DescriptorHandle
{
public:
    D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU;
};

class DescriptorManager {
public:

    bool Initialize();

    UINT ResisterTexture(ID3D12Resource* texture, DXGI_FORMAT format);
    UINT ResisterConstantBuffer(ID3D12Resource* buffer);
    
    // レイトレーシング用テクスチャ配列管理
    UINT RegisterRaytracingTexture(ID3D12Resource* texture, DXGI_FORMAT format);
    UINT GetRaytracingTextureArrayBaseIndex() const { return _raytracingTextureBaseIndex; }
    UINT GetRaytracingTextureCount() const { return _raytracingTextureCount; }

    // レイトレーシング用 頂点位置バッファ配列（StructuredBuffer<float3>）管理
    void ResetRaytracingVertexPositionArray() { _raytracingVertexPositionCount = 0; }
    UINT RegisterRaytracingVertexPositionBuffer(ID3D12Resource* vertexPositionBuffer, UINT vertexCount);
    UINT GetRaytracingVertexPositionArrayBaseIndex() const { return _raytracingVertexPositionBaseIndex; }
    UINT GetRaytracingVertexPositionCount() const { return _raytracingVertexPositionCount; }

    // レイトレーシング用 頂点法線バッファ配列（StructuredBuffer<float3>）管理
    void ResetRaytracingVertexNormalArray() { _raytracingVertexNormalCount = 0; }
    UINT RegisterRaytracingVertexNormalBuffer(ID3D12Resource* vertexNormalBuffer, UINT vertexCount);
    UINT GetRaytracingVertexNormalArrayBaseIndex() const { return _raytracingVertexNormalBaseIndex; }
    UINT GetRaytracingVertexNormalCount() const { return _raytracingVertexNormalCount; }
    
    void BindDescriptorHeaps();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle();
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandleAt(UINT index);  // 追加: インデックス指定でハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandleAt(UINT index);  // 追加: インデックス指定でハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() {
        return _cpuHandleStart;
    }

    UINT AllocateDescriptor() {
        if (_currentSVRIndex >= MAX_DESCRIPTORS) {
            SDL_Log("Descriptor heap is full!");
            return static_cast<UINT>(-1);
        }
        return _currentSVRIndex++;
	}

private:
    static DescriptorManager* s_instance;
    ComPtr<ID3D12DescriptorHeap> _heap;
    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandleStart;
    UINT _descriptorSize;
    UINT _currentSVRIndex = 0;
    static const UINT MAX_DESCRIPTORS = 1000;
    
    // レイトレーシング用テクスチャ配列の領域（連続して配置）
    // 物理的には100番から開始し、シェーダーではDescriptor Table経由でアクセス
    static const UINT RAYTRACING_TEXTURE_ARRAY_START = 100;
    static const UINT RAYTRACING_TEXTURE_ARRAY_SIZE = 16;
    UINT _raytracingTextureBaseIndex = RAYTRACING_TEXTURE_ARRAY_START;
    UINT _raytracingTextureCount = 0;

    // レイトレーシング用 頂点位置バッファ配列の領域（連続して配置）
    // 物理的には200番から開始
    static const UINT RAYTRACING_VERTEX_POSITION_ARRAY_START = 200;
    static const UINT RAYTRACING_VERTEX_POSITION_ARRAY_SIZE = 256;
    UINT _raytracingVertexPositionBaseIndex = RAYTRACING_VERTEX_POSITION_ARRAY_START;
    UINT _raytracingVertexPositionCount = 0;

    // レイトレーシング用 頂点法線バッファ配列の領域（連続して配置）
    // 物理的には頂点位置配列の直後（456番）から開始
    static const UINT RAYTRACING_VERTEX_NORMAL_ARRAY_START = RAYTRACING_VERTEX_POSITION_ARRAY_START + RAYTRACING_VERTEX_POSITION_ARRAY_SIZE;
    static const UINT RAYTRACING_VERTEX_NORMAL_ARRAY_SIZE = 256;
    UINT _raytracingVertexNormalBaseIndex = RAYTRACING_VERTEX_NORMAL_ARRAY_START;
    UINT _raytracingVertexNormalCount = 0;

public:

    static void CreateInstance() {
        DestroyInstance();
        s_instance = new DescriptorManager();
    }

    static void DestroyInstance() {
        if (s_instance) {
            delete s_instance;
            s_instance = nullptr;
        }
    }

    static DescriptorManager& Instance() {
        return *s_instance;
    }
};


#define DESCRIPTORMANAGER DescriptorManager::Instance()

class DescriptorHeap
{
public:
    DescriptorHeap();
    ID3D12DescriptorHeap* GetHeap();

    void Register(class Texture2D* texture);
    ID3D12DescriptorHeap* GetDescriptorHeap() const { return _heap.Get(); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const;

private:
    bool CreateDescriptorHeap();
    bool CreateShaderResourceView(const TexMetadata& metadata);

    ComPtr<ID3D12DescriptorHeap> _heap;

};