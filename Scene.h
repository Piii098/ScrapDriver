#pragma once
#include "DirectX.h"
#include "Framework.h"
#include "ShaderStruct.h"
#include "DescriptorHeap.h"

class Scene {
public:
	
	bool Initialize();
	void Update();
	void Render();

private:

	class VertexBuffer* _vertexBuffer;
	class ConstantBuffer* _constantBuffer[Direct3D::FRAME_BUFFER_COUNT];
	class RootSignature* _rootSignature;
	class PipelineState* _pipelineState;
	class IndexBuffer* _indexBuffer;

	std::vector<Mesh> _meshes;
	std::vector<VertexBuffer*> _vertexBuffers;
	std::vector<IndexBuffer*> _indexBuffers;
	DescriptorHeap* _descriptorHeap;
	std::vector<DescriptorHandle*> _materialHandles;

	float _rotationY = 0.0f;
};