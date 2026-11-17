#include "DirectX/Texture2D.h"
#include "DirectX/DescriptorHeap.h"
#include <map>

Texture2D::Texture2D(const std::wstring& filePath)
    : _isValid(false)
	, _svrIndex(0)
{
    _isValid = LoadFromFile(filePath);
}

Texture2D::Texture2D()
    : _isValid(false)
{
}

Texture2D::Texture2D(ID3D12Resource* resource)
    : _isValid(false)
    , _texture(resource)
{
    if (_texture) {
        _isValid = true;
        _svrIndex = DESCRIPTORMANAGER.ResisterTexture(_texture.Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
        // レイトレーシング用にも登録
        _raytracingIndex = DESCRIPTORMANAGER.RegisterRaytracingTexture(_texture.Get(), DXGI_FORMAT_R8G8B8A8_UNORM);
        if (_raytracingIndex != static_cast<UINT>(-1)) {
            SDL_Log("White texture registered for raytracing at relative index: %d", _raytracingIndex);
        }
    }
}

Texture2D::~Texture2D()
{
}

void Texture2D::Load(const std::string& assetName)
{
	std::wstring wFilePath(assetName.begin(), assetName.end());
	LoadFromFile(wFilePath);
}

std::wstring FileExtension(const std::wstring& path)
{
    auto idx = path.rfind(L'.');
    return path.substr(idx + 1, path.length() - idx - 1);
}

bool Texture2D::LoadFromFile(const std::wstring& filePath)
{
    TexMetadata metadata = {};
    ScratchImage scratchImg = {};

    auto ext = FileExtension(filePath);

    HRESULT hr = S_FALSE;

    if (ext == L"png" || ext == L"jpg")
    {
        // 戻り値を受け取らないと hr が S_FALSE のままで失敗扱いになる
        hr = LoadFromWICFile(filePath.c_str(), WIC_FLAGS_NONE, &metadata, scratchImg);
    }
    else if (ext == L"tga") // tgaの時はTGAFileを使う
    {
        hr = LoadFromTGAFile(filePath.c_str(), &metadata, scratchImg);
    }

    if (FAILED(hr)) {
        printf("Error: Failed to load WIC file. HRESULT = 0x%08X\n", hr);
        return false;
    }

    auto img = scratchImg.GetImage(0, 0, 0);
    if (!img) {
        printf("Error: Failed to get image data\n");
        return false;
    }

    if (!CreateTexture(metadata, img)) {
        return false;
    }

    _svrIndex = DESCRIPTORMANAGER.ResisterTexture(_texture.Get(), metadata.format);

    // 登録失敗チェックを追加
    if (_svrIndex == static_cast<UINT>(-1)) {
        SDL_Log("Failed to register texture");
        return false;
    }

    SDL_Log("Texture registered at index: %d", _svrIndex);
    
    // レイトレーシング用にも登録（連続配列として）
    _raytracingIndex = DESCRIPTORMANAGER.RegisterRaytracingTexture(_texture.Get(), metadata.format);
    if (_raytracingIndex != static_cast<UINT>(-1)) {
        SDL_Log("Texture also registered for raytracing at relative index: %d", _raytracingIndex);
    }

    _isValid = true;
    return true;
}


bool Texture2D::CreateTexture(const TexMetadata& metadata, const Image* img) {
    D3D12_HEAP_PROPERTIES heapprop = {};
    heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
    heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    heapprop.CreationNodeMask = 0;
    heapprop.VisibleNodeMask = 0;

    D3D12_RESOURCE_DESC desc = {};
    desc.Format = metadata.format;
    desc.Width = metadata.width;
    desc.Height = metadata.height;
    desc.DepthOrArraySize = metadata.arraySize;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.MipLevels = metadata.mipLevels;
    desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    auto hr = DIRECT3D.Device().CreateCommittedResource(
        &heapprop,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&_texture)
    );

    if (FAILED(hr)) {
        printf("Error: Failed to create texture resource. HRESULT = 0x%08X\n", hr);
        return false;
    }

    hr = _texture->WriteToSubresource(
        0,
        nullptr,
        img->pixels,
        img->rowPitch,
        img->slicePitch
    );

    if (FAILED(hr)) {
        printf("Error: Failed to write texture data. HRESULT = 0x%08X\n", hr);
        return false;
    }

    return true;
}

Texture2D* Texture2D::Get(const std::wstring& filePath) {
    // キャッシュチェック
    static std::map<std::wstring, Texture2D*> textureCache;
    
    auto it = textureCache.find(filePath);
    if (it != textureCache.end()) {
        SDL_Log("Using cached texture for: %ls (raytracing index: %d)", filePath.c_str(), it->second->GetRaytracingIndex());
        return it->second;
    }
    
    auto tex = new Texture2D(filePath);
    if (!tex->IsValid()) {
        SDL_Log("Failed to load texture: %ls, using white texture", filePath.c_str());
        delete tex;
        return GetWhite();
    }
    
    // キャッシュに登録
    textureCache[filePath] = tex;
    SDL_Log("New texture cached: %ls (raytracing index: %d)", filePath.c_str(), tex->GetRaytracingIndex());
    return tex;
}

Texture2D* Texture2D::Get(const std::string& filePath) {
    std::wstring wFilePath(filePath.begin(), filePath.end());
    return Get(wFilePath);
}

Texture2D* Texture2D::GetWhite() {
    static Texture2D* whiteTex = nullptr;
    
    // シングルトンパターンで白テクスチャを1度だけ作成
    if (whiteTex != nullptr) {
        return whiteTex;
    }
    
    ID3D12Resource* whiteTexResource = GetDefaultResource(4, 4);
    if (!whiteTexResource) {
        SDL_Log("Failed to create white texture resource.");
        return nullptr;
    }

    std::vector<unsigned char> whiteData(4 * 4 * 4);
    std::fill(whiteData.begin(), whiteData.end(), 0xff);

    if(FAILED(whiteTexResource->WriteToSubresource(
        0,
        nullptr,
        whiteData.data(),
        4 * 4,
        static_cast<UINT>(whiteData.size())
    ))) {
        SDL_Log("Failed to write white texture data.");
        whiteTexResource->Release();
        return nullptr;
    }

    whiteTex = new Texture2D(whiteTexResource);
    if (!whiteTex->IsValid()) {
        SDL_Log("Failed to create white Texture2D object.");
        delete whiteTex;
        whiteTex = nullptr;
        return nullptr;
    }
    
    SDL_Log("White texture created at index: %u", whiteTex->GetDescriptorIndex());
    return whiteTex;
}

ID3D12Resource* Texture2D::GetDefaultResource(size_t width, size_t height) {
    auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        static_cast<UINT64>(width),
        static_cast<UINT>(height),
        1, // array size
        1  // mip levels
    );

    auto texHeapProp = CD3DX12_HEAP_PROPERTIES(
        D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
        D3D12_MEMORY_POOL_L0
    );

    ID3D12Resource* buffer = nullptr;
    if (FAILED(DIRECT3D.Device().CreateCommittedResource(
        &texHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(&buffer)))) {
        SDL_Log("Failed to create default texture resource.");
        return nullptr;
    }

    return buffer;
}

//bool Texture2D::CreateDescriptorHeap() {
//    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
//    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
//    heapDesc.NodeMask = 0;
//    heapDesc.NumDescriptors = 2;
//    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
//
//    auto hr = DIRECT3D.Device().CreateDescriptorHeap(
//        &heapDesc,
//        IID_PPV_ARGS(&_texDescHeap)
//    );
//
//    if (FAILED(hr)) {
//        printf("Error: Failed to create descriptor heap. HRESULT = 0x%08X\n", hr);
//        return false;
//    }
//
//    return true;
//}
//
//bool Texture2D::CreateShaderResourceView(const TexMetadata& metadata) {
//    _srvDesc = {};
//    _srvDesc.Format = metadata.format;
//    _srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//    _srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//    _srvDesc.Texture2D.MipLevels = 1;
//
//    DIRECT3D.Device().CreateShaderResourceView(
//        _texture.Get(),
//        &_srvDesc,
//        _texDescHeap->GetCPUDescriptorHandleForHeapStart()
//	);
//
//    return true;
//}
//

void Texture2D::Bind() {
    // 有効性チェック
    if (!_isValid) {
        SDL_Log("Error: Trying to bind invalid texture");
        return;
    }

    // インデックスの妥当性チェック
    if (_svrIndex == static_cast<UINT>(-1)) {
        SDL_Log("Error: Invalid texture descriptor index");
        return;
    }

    // GetGPUHandleAt を使用して直接ハンドルを取得
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = DESCRIPTORMANAGER.GetGPUHandleAt(_svrIndex);

    //SDL_Log("Binding texture at index: %d, handle: %llu", _svrIndex, srvHandle.ptr);

    // テクスチャはルートパラメータ0にバインド（定数バッファが1を使用）
    DIRECT3D.CommandList().SetGraphicsRootDescriptorTable(0, srvHandle);
}