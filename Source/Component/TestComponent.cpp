#include "Component/TestComponent.h"

TestComponent1::TestComponent1(class GameObject& parent, int updateLayer)
	: Component(parent, updateLayer) {

	
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    Vector3 vertices[] = {
        {-0.4f,-0.7f,0.0f} ,//左下
        {-0.4f,0.7f,0.0f} ,//左上
        {0.4f,-0.7f,0.0f} ,//右下
        {0.4f,0.7f,0.0f} ,//右上
    };

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = sizeof(vertices);
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if(FAILED(DIRECT3D.Device().CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_vertexBuffer)))) {
            SDL_Log("Failed to create vertex buffer resource.");
            return;
    }

    Vector3* vertMap = nullptr;
    if(FAILED(_vertexBuffer->Map(0, nullptr, (void**)&vertMap))) {
        SDL_Log("Failed to map vertex buffer.");
        return;
    }

    std::copy(std::begin(vertices), std::end(vertices), vertMap);
    _vertexBuffer->Unmap(0, nullptr);

    _vbView = {};
    _vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vbView.SizeInBytes = sizeof(vertices);
    _vbView.StrideInBytes = sizeof(Vector3);

    unsigned short indices[] = {
        0,1,2, 
        2,1,3
    };

    //設定は、バッファのサイズ以外頂点バッファの設定を使いまわして
    //OKだと思います。
    desc.Width = sizeof(indices);
    if(FAILED(DIRECT3D.Device().CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_indexBuffer)))) {
            SDL_Log("Failed to create index buffer resource.");
            return;
	}
    //作ったバッファにインデックスデータをコピー
    unsigned short* mappedIdx = nullptr;
    _indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
    std::copy(std::begin(indices), std::end(indices), mappedIdx);
    _indexBuffer->Unmap(0, nullptr);

    //インデックスバッファビューを作成
    _ibView = {};
    _ibView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
    _ibView.Format = DXGI_FORMAT_R16_UINT;
    _ibView.SizeInBytes = sizeof(indices);

}

TestComponent1::~TestComponent1() {
}

void TestComponent1::Render() {
	//SDL_Log("TestComponent Render \n");
	DIRECT3D.CommandList().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DIRECT3D.CommandList().IASetVertexBuffers(0, 1, &_vbView); // ver

	DIRECT3D.CommandList().IASetIndexBuffer(&_ibView); // ind

	DIRECT3D.CommandList().DrawIndexedInstanced(6, 1, 0, 0, 0);
}

TestComponent2::TestComponent2(GameObject& parent)
    : Component(parent, 100)
    , _isValid(false) {
}

TestComponent2::~TestComponent2() {
}

bool TestComponent2::Initialize(float* vertices, int vertexCount) {
    // レイトレーシングマネージャーに三角形を追加
    // 注: RaytracingManagerはシングルトンまたはRendererManagerから取得
    // ここでは簡略化のため直接呼び出し

    _isValid = true;
    return true;
}

void TestComponent2::Render() {
    if (!_isValid) return;

    // レイトレーシングのレンダリングはRaytracingManagerが行う
    // このコンポーネントは加速構造の更新などを担当
}