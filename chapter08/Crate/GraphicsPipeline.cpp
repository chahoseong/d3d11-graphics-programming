#include "GraphicsPipeline.h"
#include "RenderStates.h"

#include <iostream>

#include <d3dcompiler.h>


using namespace DirectX;
using namespace Microsoft::WRL;

GraphicsPipeline::GraphicsPipeline(ID3D11Device* device)
	: md3dDevice(device)
{
}

Microsoft::WRL::ComPtr<ID3DBlob> GraphicsPipeline::CompileShader(const std::wstring& filename, const std::string& target) const
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errorMsg;

	// compile and create vertex shader
	HRESULT hr = D3DCompileFromFile(
		filename.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		target.c_str(),
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr))
	{
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return nullptr;
	}

	return compiledShader;
}

ComPtr<ID3D11VertexShader> GraphicsPipeline::CreateVertexShader(const std::wstring& filename, ComPtr<ID3DBlob>* outCompiledShader)
{
	ComPtr<ID3DBlob> compiled = CompileShader(filename, "vs_5_0");
	if (!compiled) {
		return nullptr;
	}

	if (outCompiledShader) {
		*outCompiledShader = compiled;
	}
	
	ComPtr<ID3D11VertexShader> vs;
	md3dDevice->CreateVertexShader(compiled->GetBufferPointer(), compiled->GetBufferSize(), nullptr, vs.GetAddressOf());

	return vs;

}

ComPtr<ID3D11PixelShader> GraphicsPipeline::CreatePixelShader(const std::wstring& filename, ComPtr<ID3DBlob>* outCompiledShader)
{
	ComPtr<ID3DBlob> compiled = CompileShader(filename, "ps_5_0");
	if (!compiled) {
		return nullptr;
	}

	if (outCompiledShader) {
		*outCompiledShader = compiled;
	}

	ComPtr<ID3D11PixelShader> ps;
	md3dDevice->CreatePixelShader(compiled->GetBufferPointer(), compiled->GetBufferSize(), nullptr, ps.GetAddressOf());
	
	return ps;
}


BasicGraphicsPipeline::BasicGraphicsPipeline(ID3D11Device* device)
	: GraphicsPipeline(device), mFrameCB(device), mObjectCB(device)
{
	LoadShaders();
}

void BasicGraphicsPipeline::SetWorld(CXMMATRIX M)
{
	XMStoreFloat4x4(&mObjectCB.data.world, XMMatrixTranspose(M));
}

void BasicGraphicsPipeline::SetWorldInvTranspose(CXMMATRIX M)
{
	XMStoreFloat4x4(&mObjectCB.data.worldInvTranspose, XMMatrixTranspose(M));
}

void BasicGraphicsPipeline::SetWorldViewProj(CXMMATRIX M)
{
	XMStoreFloat4x4(&mObjectCB.data.worldViewProj, XMMatrixTranspose(M));
}

void BasicGraphicsPipeline::SetTexTransform(DirectX::CXMMATRIX M)
{
	XMStoreFloat4x4(&mObjectCB.data.texTransform, XMMatrixTranspose(M));
}

void BasicGraphicsPipeline::SetEyePosW(const XMFLOAT3& v)
{
	mFrameCB.data.eyePosW = v;
}

void BasicGraphicsPipeline::SetDirLights(const DirectionalLight* lights, int count)
{
	memcpy(&mFrameCB.data.dirLights[0], lights, sizeof(DirectionalLight) * count);
	mFrameCB.data.lightCount = count;
}

void BasicGraphicsPipeline::SetMaterial(const Material& mat)
{
	mObjectCB.data.material = mat;
}

void BasicGraphicsPipeline::SetDiffuseMap(ComPtr<ID3D11ShaderResourceView> texture)
{
	mDiffuseMap = texture;
}

void BasicGraphicsPipeline::UseDiffuseMap(bool enabled)
{
	mObjectCB.data.useTexture = static_cast<int>(enabled);
}

ShaderBytecodeView BasicGraphicsPipeline::GetVertexShaderBytecode() const
{
	return { mCompiledVertexShader->GetBufferPointer(), mCompiledVertexShader->GetBufferSize() };
}

void BasicGraphicsPipeline::Apply(ID3D11DeviceContext* context) const
{
	UpdateFrameBuffer(context);
	UpdateObjectBuffer(context);
	Bind(context);
}

void BasicGraphicsPipeline::LoadShaders()
{
	mVertexShader = CreateVertexShader(L"basic.vertex.hlsl", &mCompiledVertexShader);
	mPixelShader = CreatePixelShader(L"basic.pixel.hlsl", nullptr);
}

void BasicGraphicsPipeline::UpdateFrameBuffer(ID3D11DeviceContext* context) const
{
	mFrameCB.Update(context);
}

void BasicGraphicsPipeline::UpdateObjectBuffer(ID3D11DeviceContext* context) const
{
	mObjectCB.Update(context);
}

void BasicGraphicsPipeline::Bind(ID3D11DeviceContext* context) const
{
	context->VSSetShader(mVertexShader.Get(), nullptr, 0);
	{
		ID3D11Buffer* constantBuffers[] = { mObjectCB.handle.Get()};
		context->VSSetConstantBuffers(ObjectBufferSlot, 1, constantBuffers);
	}

	context->PSSetShader(mPixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, RenderStates::AnisotropicSS.GetAddressOf());
	context->PSSetShaderResources(0, 1, mDiffuseMap.GetAddressOf());
	{
		ID3D11Buffer* constantBuffers[] = { mFrameCB.handle.Get(), mObjectCB.handle.Get() };
		context->PSSetConstantBuffers(FrameBufferSlot, 2, constantBuffers);
	}
}


BasicGraphicsPipeline* Pipelines::Basic = nullptr;

void Pipelines::InitAll(ID3D11Device* device)
{
	Basic = new BasicGraphicsPipeline(device);
}

void Pipelines::DestroyAll()
{
	delete Basic;
}
