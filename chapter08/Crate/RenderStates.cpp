#include "RenderStates.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11SamplerState> RenderStates::AnisotropicSS;

void RenderStates::InitAll(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC anisotropicDesc;
	ZeroMemory(&anisotropicDesc, sizeof(D3D11_SAMPLER_DESC));
	anisotropicDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	anisotropicDesc.MaxAnisotropy = 4;
	anisotropicDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	//anisotropicDesc.MinLOD = 0;
	//anisotropicDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ThrowIfFailed(device->CreateSamplerState(&anisotropicDesc, AnisotropicSS.GetAddressOf()));
}

void RenderStates::DestroyAll()
{
	AnisotropicSS.Reset();
}
