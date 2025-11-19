#include "DirectX/RaytracingPipeline.h"

RaytracingPipeline::RaytracingPipeline() : _isValid(false) {
}

bool RaytracingPipeline::CompileShaders() {
    // DXCコンパイラを使用してレイトレーシングシェーダーをコンパイル
    ComPtr<IDxcUtils> dxcUtils;
    ComPtr<IDxcCompiler3> dxcCompiler;
    ComPtr<IDxcIncludeHandler> includeHandler;

    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
    if (FAILED(hr)) {
        SDL_Log("Failed to create DxcUtils");
        return false;
    }

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
    if (FAILED(hr)) {
        SDL_Log("Failed to create DxcCompiler");
        return false;
    }

    hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
    if (FAILED(hr)) {
        SDL_Log("Failed to create include handler");
        return false;
    }

    // シェーダーファイルを読み込む
    ComPtr<IDxcBlobEncoding> sourceBlob;
    hr = dxcUtils->LoadFile(L"Assets/Shader/Raytracing.hlsl", nullptr, &sourceBlob);
    if (FAILED(hr)) {
        SDL_Log("Failed to load shader file: Assets/Shader/Raytracing.hlsl");
        return false;
    }

    // コンパイル引数を設定
    std::vector<LPCWSTR> arguments = {
        L"-T", L"lib_6_3",              // ターゲットプロファイル
        L"-Zi",                          // デバッグ情報を含める
        L"-Od",                          // 最適化を無効化
        L"-Qembed_debug",                // デバッグ情報を埋め込む
        DXC_ARG_WARNINGS_ARE_ERRORS,     // 警告をエラーとして扱う
    };

    DxcBuffer sourceBuffer;
    sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
    sourceBuffer.Size = sourceBlob->GetBufferSize();
    sourceBuffer.Encoding = 0;

    // コンパイル実行
    ComPtr<IDxcResult> compileResult;
    hr = dxcCompiler->Compile(
        &sourceBuffer,
        arguments.data(),
        static_cast<UINT32>(arguments.size()),
        includeHandler.Get(),
        IID_PPV_ARGS(&compileResult));

    if (FAILED(hr)) {
        SDL_Log("DXC Compile failed");
        return false;
    }

    // コンパイル結果を確認
    HRESULT compileStatus;
    compileResult->GetStatus(&compileStatus);
    if (FAILED(compileStatus)) {
        ComPtr<IDxcBlobEncoding> errorBlob;
        compileResult->GetErrorBuffer(&errorBlob);
        if (errorBlob) {
            SDL_Log("Shader compile error: %s", (char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    // コンパイル済みシェーダーを取得
    ComPtr<IDxcBlob> shaderBlobDxc;
    hr = compileResult->GetResult(&shaderBlobDxc);
    if (FAILED(hr)) {
        SDL_Log("Failed to get shader blob");
        return false;
    }

    // IDxcBlob を ID3DBlob としてキャスト (互換性あり)
    _shaderBlob = shaderBlobDxc;

    return true;
}

bool RaytracingPipeline::Initialize(ID3D12RootSignature* globalRootSig, ID3D12RootSignature* localRootSig) {
    if (!CompileShaders()) {
        return false;
    }

    // すべての記述子を先に定義（ポインタの安定性のため）
    D3D12_EXPORT_DESC exports[3] = {};
    exports[0].Name = L"MyRaygenShader";
    exports[0].ExportToRename = nullptr;
    exports[0].Flags = D3D12_EXPORT_FLAG_NONE;
    
    exports[1].Name = L"MyClosestHitShader";
    exports[1].ExportToRename = nullptr;
    exports[1].Flags = D3D12_EXPORT_FLAG_NONE;
    
    exports[2].Name = L"MyMissShader";
    exports[2].ExportToRename = nullptr;
    exports[2].Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
    dxilLibDesc.DXILLibrary.pShaderBytecode = _shaderBlob->GetBufferPointer();
    dxilLibDesc.DXILLibrary.BytecodeLength = _shaderBlob->GetBufferSize();
    dxilLibDesc.NumExports = 3;
    dxilLibDesc.pExports = exports;

    D3D12_HIT_GROUP_DESC hitGroupDesc = {};
    hitGroupDesc.HitGroupExport = L"MyHitGroup";
    hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
    hitGroupDesc.ClosestHitShaderImport = L"MyClosestHitShader";

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    shaderConfig.MaxPayloadSizeInBytes = 5 * sizeof(float); // float4 color + uint recursionDepth = 5 floats
    shaderConfig.MaxAttributeSizeInBytes = 2 * sizeof(float);

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = 4; // 反射レイトレーシングのための再帰深度（HLSL MAX_RAY_RECURSION_DEPTH=4と一致）

    // Subobjectの配列（固定サイズで確保）
    D3D12_STATE_SUBOBJECT subobjects[5] = {};
    UINT subobjectIndex = 0;

    // [0] DXIL Library
    subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    subobjects[subobjectIndex].pDesc = &dxilLibDesc;
    UINT dxilLibIndex = subobjectIndex++;

    // [1] Hit Group
    subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    subobjects[subobjectIndex].pDesc = &hitGroupDesc;
    subobjectIndex++;

    // [2] Shader Config
    subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    subobjects[subobjectIndex].pDesc = &shaderConfig;
    subobjectIndex++;

    // [3] Global Root Signature
    subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    subobjects[subobjectIndex].pDesc = &globalRootSig;
    subobjectIndex++;

    // [4] Pipeline Config
    subobjects[subobjectIndex].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    subobjects[subobjectIndex].pDesc = &pipelineConfig;
    subobjectIndex++;

    SDL_Log("Creating RTPSO with %d subobjects:", subobjectIndex);
    SDL_Log("  [0] DXIL Library (3 exports)");
    SDL_Log("  [1] Hit Group");
    SDL_Log("  [2] Shader Config");
    SDL_Log("  [3] Global Root Signature");
    SDL_Log("  [4] Pipeline Config");

    // RTPSO作成
    D3D12_STATE_OBJECT_DESC stateObjectDesc = {};
    stateObjectDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    stateObjectDesc.NumSubobjects = subobjectIndex;
    stateObjectDesc.pSubobjects = subobjects;

    HRESULT hr = DIRECT3D.DXRDevice().CreateStateObject(&stateObjectDesc, IID_PPV_ARGS(&_stateObject));
    if (FAILED(hr)) {
        SDL_Log("Failed to create raytracing state object (HRESULT: 0x%08X)", hr);
        return false;
    }

    // StateObjectProperties を取得
    hr = _stateObject->QueryInterface(IID_PPV_ARGS(&_stateObjectProps));
    if (FAILED(hr)) {
        SDL_Log("Failed to get state object properties");
        return false;
    }

    // デバッグ: 利用可能なエクスポート名をすべてログ出力
    SDL_Log("=== Available Shader Exports ===");
    const WCHAR* exportNames[] = {
        L"MyRaygenShader",
        L"MyMissShader",
        L"MyHitGroup"
    };
    
    for (const auto& name : exportNames) {
        void* shaderId = _stateObjectProps->GetShaderIdentifier(name);
        if (shaderId) {
            SDL_Log("  ✓ Found: %ls", name);
        } else {
            SDL_Log("  ✗ Missing: %ls", name);
        }
    }
    SDL_Log("================================");

    _isValid = true;
    return true;
}