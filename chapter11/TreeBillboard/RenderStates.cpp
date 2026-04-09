#include "RenderStates.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11RasterizerState> RenderStates::WireframeRS;
ComPtr<ID3D11RasterizerState> RenderStates::NoCullRS;
ComPtr<ID3D11RasterizerState> RenderStates::CullClockwiseRS;

ComPtr<ID3D11BlendState> RenderStates::AlphaToCoverageBS;
ComPtr<ID3D11BlendState> RenderStates::TransparentBS;
ComPtr<ID3D11BlendState> RenderStates::NoRenderTargetWritesBS;

ComPtr<ID3D11DepthStencilState> MarkMirrorDSS;
ComPtr<ID3D11DepthStencilState> DrawReflectionDSS;
ComPtr<ID3D11DepthStencilState> NoDoubleBlendDSS;

ComPtr<ID3D11SamplerState> RenderStates::AnisotropicSS;

void RenderStates::InitAll(ID3D11Device* device)
{
	/*
	 * Rasterizer States
	 */

	// Wireframe
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&wireframeDesc, WireframeRS.GetAddressOf()));

	// No Cull
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&noCullDesc, NoCullRS.GetAddressOf()));

	// Cull clockwise
	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&cullClockwiseDesc, CullClockwiseRS.GetAddressOf()));

	/*
	 * Blend States
	 */

	// Alpha to Coverage
	D3D11_BLEND_DESC alphaToCoverageDesc;
	ZeroMemory(&alphaToCoverageDesc, sizeof(D3D11_BLEND_DESC));
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ThrowIfFailed(device->CreateBlendState(&alphaToCoverageDesc, AlphaToCoverageBS.GetAddressOf()));

	// Transparent
	D3D11_BLEND_DESC transparentDesc;
	ZeroMemory(&transparentDesc, sizeof(D3D11_BLEND_DESC));
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ThrowIfFailed(device->CreateBlendState(&transparentDesc, TransparentBS.GetAddressOf()));
	
	// No render target writes
	D3D11_BLEND_DESC noRenderTargetWritesDesc;
	ZeroMemory(&noRenderTargetWritesDesc, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	ThrowIfFailed(device->CreateBlendState(&noRenderTargetWritesDesc, NoRenderTargetWritesBS.GetAddressOf()));


	// Anisotropic Sampler State
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
}

void RenderStates::DestroyAll()
{
	WireframeRS.Reset();
	NoCullRS.Reset();
	CullClockwiseRS.Reset();

	AlphaToCoverageBS.Reset();
	TransparentBS.Reset();
	NoRenderTargetWritesBS.Reset();

	AnisotropicSS.Reset();
}
