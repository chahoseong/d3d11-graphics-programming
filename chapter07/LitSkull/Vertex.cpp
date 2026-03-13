#include "Vertex.h"

using namespace DirectX;
using namespace Microsoft::WRL;

const std::array<D3D11_INPUT_ELEMENT_DESC, 2> InputLayoutDesc::PosNormal = { {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
} };

ComPtr<ID3D11InputLayout> InputLayouts::PosNormal;

void InputLayouts::InitAll(ID3D11Device* device, ShaderBytecodeView basicVertexShaderBytecode)
{
	device->CreateInputLayout(InputLayoutDesc::PosNormal.data(),
		InputLayoutDesc::PosNormal.size(),
		basicVertexShaderBytecode.pointer,
		basicVertexShaderBytecode.size,
		PosNormal.GetAddressOf());
}

void InputLayouts::DestroyAll()
{
	PosNormal.Reset();
}
