#pragma once
#include "DirectX/DirectX.h"
#include <string>

class Texture2D
{
public:
    Texture2D(const std::wstring& filePath);
    Texture2D();
	Texture2D(ID3D12Resource* resource);
    ~Texture2D();

    void Load(const std::string& filePath);
    bool IsValid() const { return _isValid; }
    ID3D12Resource* GetTexture() const { return _texture.Get(); }
    UINT GetDescriptorIndex() const { return _svrIndex; }  // インデックス取得用

	static Texture2D* Get(const std::wstring& filePath);
	static Texture2D* Get(const std::string& filePath); 
    static Texture2D* GetWhite();
	static ID3D12Resource* GetDefaultResource(size_t width, size_t height);

    void Bind();

private:
    bool LoadFromFile(const std::wstring& filePath);
    bool CreateTexture(const TexMetadata& metadata, const Image* image);

    bool _isValid;
    UINT _svrIndex;
    ComPtr<ID3D12Resource> _texture;
};