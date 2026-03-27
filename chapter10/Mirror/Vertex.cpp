#include "Vertex.h"

using namespace DirectX;
using namespace Microsoft::WRL;

const std::array<D3D11_INPUT_ELEMENT_DESC, 3> InputLayoutDesc::Basic32 = { {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
} };

ComPtr<ID3D11InputLayout> InputLayouts::Basic32;

void InputLayouts::InitAll(ID3D11Device* device, ShaderBytecodeView basicVertexShaderBytecode)
{
	device->CreateInputLayout(InputLayoutDesc::Basic32.data(),
		InputLayoutDesc::Basic32.size(),
		basicVertexShaderBytecode.pointer,
		basicVertexShaderBytecode.size,
		Basic32.GetAddressOf());
}

void InputLayouts::DestroyAll()
{
	Basic32.Reset();
}
