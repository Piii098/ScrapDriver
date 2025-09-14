#pragma once
#include "Framework.h"
#include "DirectX.h"
#include "../Utility/Math.h"

struct Vertex {
	Vector3 Position; // �ʒu���W
	Vector3 Normal; // �@��
	Vector2 UV; // uv���W
	Vector3 Tangent; // �ڋ��
	Vector4 Color; // ���_�F

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 5;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) Transform
{
	Matrix4x4 World; // ���[���h�s��
	Matrix4x4 View; // �r���[�s��
	Matrix4x4 Proj; // ���e�s��
};

struct Mesh
{
	std::vector<Vertex> Vertices; // ���_�f�[�^�̔z��
	std::vector<uint32_t> Indices; // �C���f�b�N�X�̔z��
	std::wstring DiffuseMap; // �e�N�X�`���̃t�@�C���p�X
};