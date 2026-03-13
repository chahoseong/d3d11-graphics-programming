#pragma once

#include <array>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

namespace Vertex
{
	struct PosNormal
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
	};
}

class InputLayoutDesc
{
public:
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> PosNormal;
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> PosNormal;
};