#pragma once
#include "DirectX/DirectX.h"
#include "Core/Component.h"
#include "Framework.h"

// レンダー機能を試すためのコンポーネント
class TestComponent1 : public Component {
public:
	TestComponent1(class GameObject& parent, int updateLayer = 100);
	~TestComponent1();

	void Render() override;

private:
	ComPtr<ID3D12Resource> _vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vbView;

	ComPtr<ID3D12Resource> _indexBuffer;
	D3D12_INDEX_BUFFER_VIEW _ibView;
};

// レイトレーシング機能を試すためのコンポーネント
class TestComponent2 : public Component {
public:
	TestComponent2(GameObject& parent);
    ~TestComponent2();

    bool Initialize(float* vertices, int vertexCount);

    void Render() override;

    bool IsValid() const { return _isValid; }

private:
    bool _isValid = false;
};