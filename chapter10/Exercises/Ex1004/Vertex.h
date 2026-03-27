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
}

class InputLayoutDesc
{
public:
	// Describes how PosNormal is read by the input-assembler stage.
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 3> Basic32;
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device, ShaderBytecodeView basicVertexShaderBytecode);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> Basic32;
};
