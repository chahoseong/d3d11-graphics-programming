#pragma once

#include "GraphicsPipeline.h"

#include <array>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

namespace Vertex
{
	struct Basic32
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 Tex;
	};

	struct Pos
	{
		DirectX::XMFLOAT3 Pos;
	};
}

class InputLayoutDesc
{
public:
	// Describes how PosNormal is read by the input-assembler stage.
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 3> Basic32;
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 1> Pos;
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> Basic32;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> Pos;
};
