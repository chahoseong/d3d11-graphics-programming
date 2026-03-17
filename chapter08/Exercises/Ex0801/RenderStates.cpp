#include "RenderStates.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11SamplerState> RenderStates::PointSS;
ComPtr<ID3D11SamplerState> RenderStates::LinearSS;
ComPtr<ID3D11SamplerState> RenderStates::AnisotropicSS;
ComPtr<ID3D11SamplerState> RenderStates::WarpSS;
ComPtr<ID3D11SamplerState> RenderStates::ClampSS;
ComPtr<ID3D11SamplerState> RenderStates::MirrorSS;

void RenderStates::InitAll(ID3D11Device* device)
{
	D3D11_SAMPLER_DESC anisotropicDesc;
	ZeroMemory(&anisotropicDesc, sizeof(D3D11_SAMPLER_DESC));
	anisotropicDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	anisotropicDesc.MaxAnisotropy = 4;
	anisotropicDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	anisotropicDesc.MinLOD = 0;
	anisotropicDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ThrowIfFailed(device->CreateSamplerState(&anisotropicDesc, AnisotropicSS.GetAddressOf()));

	D3D11_SAMPLER_DESC linearDesc;
	ZeroMemory(&linearDesc, sizeof(D3D11_SAMPLER_DESC));
	linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	linearDesc.MinLOD = 0;
	linearDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ThrowIfFailed(device->CreateSamplerState(&linearDesc, LinearSS.GetAddressOf()));

	D3D11_SAMPLER_DESC pointDesc;
	ZeroMemory(&pointDesc, sizeof(D3D11_SAMPLER_DESC));
	pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pointDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	pointDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	pointDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	pointDesc.MinLOD = 0;
	pointDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ThrowIfFailed(device->CreateSamplerState(&pointDesc, PointSS.GetAddressOf()));

	D3D11_SAMPLER_DESC warpDesc;
	ZeroMemory(&warpDesc, sizeof(D3D11_SAMPLER_DESC));
	warpDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	warpDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	warpDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	warpDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	ThrowIfFailed(device->CreateSamplerState(&warpDesc, WarpSS.GetAddressOf()));

	D3D11_SAMPLER_DESC clampDesc;
	ZeroMemory(&clampDesc, sizeof(D3D11_SAMPLER_DESC));
	clampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	clampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ThrowIfFailed(device->CreateSamplerState(&clampDesc, ClampSS.GetAddressOf()));

	D3D11_SAMPLER_DESC mirrorDesc;
	ZeroMemory(&mirrorDesc, sizeof(D3D11_SAMPLER_DESC));
	mirrorDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	mirrorDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	mirrorDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	mirrorDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	ThrowIfFailed(device->CreateSamplerState(&mirrorDesc, MirrorSS.GetAddressOf()));
}

void RenderStates::DestroyAll()
{
	AnisotropicSS.Reset();
}
