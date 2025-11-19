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