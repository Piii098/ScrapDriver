#pragma once
#include "Core/Component.h"
#include "Framework.h"
#include "DirectX/ShaderStruct.h"

class ModelComponent : public Component {
public:
	ModelComponent(class GameObject& parent, int updateLayer = 100);
	~ModelComponent();
	bool LoadModel(const std::string& name);
	void Render();

private:
	//std::shared_ptr<class Model> _model;
	std::vector<Mesh> _meshes;
	std::vector<class VertexBuffer*> _vertexBuffers;
	std::vector<class IndexBuffer*> _indexBuffers;
	std::vector<class Texture2D*> _textures;
	std::vector<UINT> _textureIndices;
};