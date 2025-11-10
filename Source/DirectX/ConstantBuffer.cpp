#include "DirectX/ConstantBuffer.h"
#include "DirectX/DirectX.h"
#include "DirectX/DescriptorHeap.h"

ConstantBuffer::ConstantBuffer()
{
    _constantBuffer = nullptr;
    // 定数バッファ用のヒーププロパティを設定
    D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

    auto desc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(Transform) + 0xff) & ~0xff);

    if(FAILED(DIRECT3D.Device().CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(_constantBuffer.GetAddressOf())))) {
            SDL_Log("Failed to create constant buffer resource.");
			_mappedPtr = nullptr;   
            _isValid = false;
            return;
	}

    _constantBuffer->Map(0, nullptr, (void**)&_mappedPtr);

    //DESCRIPTORMANAGER.ResisterConstantBuffer(_constantBuffer.Get());

	_isValid = true;
}

bool ConstantBuffer::IsValid()
{
    return _isValid;
}

Transform* ConstantBuffer::GetPtr() const
{
    return _mappedPtr;
}
