#pragma once

#include "GraphicsPipeline.h"

#include <array>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

namespace Vertex
{
	// CPU-side vertex layout used by the basic lighting chapter.
	struct PosNormal
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
	};
}

class InputLayoutDesc
{
public:
	// Describes how PosNormal is read by the input-assembler stage.
	static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> PosNormal;
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device, ShaderBytecodeView basicVertexShaderBytecode);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11InputLayout> PosNormal;
};
