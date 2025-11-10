#pragma once
#include "Core/Component.h"
#include "DirectX/DirectX.h"

struct TexRGBA {
	unsigned char R, G, B, A;
};

class PlaneComponent : public Component {
public:
	PlaneComponent(class GameObject& parent, int updateLayer = 100);
	~PlaneComponent();

	void Render() override;

private:
	class VertexBuffer* _vertexBuffer;
	class IndexBuffer* _indexBuffer;
	class Texture2D* _texture2D;
	ComPtr<ID3D12Resource> _texture;
	ComPtr<ID3D12DescriptorHeap> _texDescHeap;


};