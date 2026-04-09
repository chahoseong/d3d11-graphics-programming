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

		Basic32() = default;
		Basic32(const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv)
			: Pos(p), Normal(n), Tex(uv) { }
		Basic32(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: Pos(px, py, pz), Normal(nx, ny, nz), Tex(u, v) {  }
	};

	struct TreePointSprite
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT2 Size;
	};
}

class InputLayoutDesc
{
public:
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 3> Basic32;
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> TreePointSprite;
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> Basic32;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> TreePointSprite;
};
