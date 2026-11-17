#include "Component/PlaneComponent.h"
#include "Core/GameObject.h"
#include "DirectX/VertexBuffer.h"
#include "DirectX/IndexBuffer.h"
#include "DirectX/Texture2D.h"

PlaneComponent::PlaneComponent(class GameObject& parent, int updateLayer)
	: Component(parent, updateLayer) {

	Vertex vertices[4];
	vertices[0].Position = { -1.f,-1.f,0.0f }; vertices[0].UV = { 0.0f,1.0f };//左下
	vertices[1].Position = { -1.f, 1.f,0.0f }; vertices[1].UV = { 0.0f,0.0f };//左上
	vertices[2].Position = { 1.f,-1.f,0.0f }; vertices[2].UV = { 1.0f,1.0f };//右下
	vertices[3].Position = { 1.f, 1.f,0.0f }; vertices[3].UV = { 1.0f,0.0f };//右上

	//	= {
	//	{{-1.f,-1.f,0.0f},{0.0f,1.0f} },//左下
	//	{{-1.f,1.f,0.0f} ,{0.0f,0.0f}},//左上
	//	{{1.f,-1.f,0.0f} ,{1.0f,1.0f}},//右下
	//	{{1.f,1.f,0.0f} ,{1.0f,0.0f}},//右上
	//};
	
	_vertexBuffer = new VertexBuffer(vertices, _countof(vertices));

	unsigned short indices[] = {
		0,1,2,
		2,1,3
	};

	_indexBuffer = new IndexBuffer(indices, _countof(indices));


	_texture2D = new Texture2D();
	_texture2D->Load("Assets/texture.jpg");

	//TexMetadata metadata = {};
	//ScratchImage scratchImg = {};

	//LoadFromWICFile(
	//	L"Assets/texture.jpg",
	//	WIC_FLAGS_NONE,
	//	&metadata,
	//	scratchImg
	//);

	//auto img = scratchImg.GetImage(0, 0, 0);


	//D3D12_HEAP_PROPERTIES heapprop = {};
	//heapprop.Type = D3D12_HEAP_TYPE_CUSTOM;
	//heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//heapprop.CreationNodeMask = 0;
	//heapprop.VisibleNodeMask = 0;

	//D3D12_RESOURCE_DESC desc = {};
	//desc.Format = metadata.format;//RGBAフォーマット
	//desc.Width = metadata.width;
	//desc.Height = metadata.height;
	//desc.DepthOrArraySize = metadata.arraySize;
	//desc.SampleDesc.Count = 1;//通常テクスチャなのでアンチェリしない
	//desc.SampleDesc.Quality = 0;//
	//desc.MipLevels = metadata.mipLevels;
	//desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	//desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	//desc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	//DIRECT3D.Device().CreateCommittedResource(
	//	&heapprop,
	//	D3D12_HEAP_FLAG_NONE,
	//	&desc,
	//	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	//	nullptr,
	//	IID_PPV_ARGS(&_texture)
	//);

	////std::vector<TexRGBA> textureData(256 * 256);

	////for (auto& rgba : textureData) {
	////	rgba.R = rand() % 256;
	////	rgba.G = rand() % 256;
	////	rgba.B = rand() % 256;
	////	rgba.A = 255;
	////}

	//_texture->WriteToSubresource(
	//	0,
	//	nullptr, // 全領域
	//	img->pixels,
	//	img->rowPitch,
	//	img->slicePitch
	//);

	//D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	//heapDesc.NodeMask = 0;
	//heapDesc.NumDescriptors = 1;
	//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	//// デスクリプタヒープの作成にエラーチェックを追加
	//auto hr = DIRECT3D.Device().CreateDescriptorHeap(
	//	&heapDesc,
	//	IID_PPV_ARGS(&_texDescHeap)
	//);

	//if (FAILED(hr)) {
	//	printf("Error: Failed to create descriptor heap. HRESULT = 0x%08X\n", hr);
	//	return; // コンストラクタなのでreturnで処理を止める
	//}

	//if (!_texDescHeap) {
	//	printf("Error: Descriptor heap is null after creation\n");
	//	return;
	//}

	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Format = metadata.format;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MipLevels = 1;
	//DIRECT3D.Device().CreateShaderResourceView(
	//	_texture.Get(),
	//	&srvDesc,
	//	_texDescHeap->GetCPUDescriptorHandleForHeapStart()
	//);

}

PlaneComponent::~PlaneComponent() {
}

void PlaneComponent::Render() {

	DIRECT3D.CommandList().IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	_texture2D->Bind();
	_vertexBuffer->Bind();
	_indexBuffer->Bind();

	DIRECT3D.CommandList().DrawIndexedInstanced(6, 1, 0, 0, 0);
}