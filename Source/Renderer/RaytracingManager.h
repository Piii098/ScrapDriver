#pragma once
#include "DirectX/DirectX.h"
#include "DirectX/AccelerationStructure.h"
#include "DirectX/RaytracingPipeline.h"
#include "DirectX/ShaderTable.h"
#include "DirectX/RaytracingOutput.h"

class RaytracingRootSignature; // 前方宣言
class Texture2D; // 前方宣言

class RaytracingManager {
public:
    RaytracingManager();
    ~RaytracingManager();

    bool Initialize();
    void Render();

    // 三角形を追加（簡易版）
    bool AddTriangle(float* vertices, int vertexCount);
    
    // メッシュを追加（頂点とインデックス、テクスチャ付き）- メッシュIDを返す
    int AddMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices);
    
    // メッシュを更新（既存のメッシュIDを使用）
    bool UpdateMesh(int meshId, const std::vector<Vertex>& vertices, const std::vector<unsigned short>& indices);
    
    // テクスチャを設定
    void SetTexture(Texture2D* texture);

    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
    bool _needsRebuild = false; // Acceleration Structureの再構築が必要か

    std::unique_ptr<AccelerationStructure> _accelerationStructure;
    std::unique_ptr<RaytracingRootSignature> _rootSignature;
    std::unique_ptr<RaytracingPipeline> _pipeline;
    std::unique_ptr<ShaderTable> _shaderTable;
    std::unique_ptr<RaytracingOutput> _output;
    
    // 追加されたメッシュのデータを保持
    std::vector<std::vector<float>> _meshVertices;
    
    // テクスチャ
    Texture2D* _currentTexture = nullptr;
    
    // カメラ用Constant Buffer
    ComPtr<ID3D12Resource> _cameraBuffer;
    void* _cameraBufferMapped = nullptr;
    
    // Material Buffer用メンバー変数を追加
    ComPtr<ID3D12Resource> _materialBuffer;
    void* _materialBufferMapped = nullptr;
    
    bool CreateRootSignatures();
    bool CreatePipeline();
    bool CreateShaderTable();
    bool CreateOutput();
    bool CreateCameraBuffer();
    bool CreateMaterialBuffer();  // 新規追加
    void UpdateCameraBuffer();
    void UpdateMaterialBuffer(); // 新規追加
};