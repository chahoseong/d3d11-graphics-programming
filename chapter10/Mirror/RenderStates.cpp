#include "RenderStates.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

ComPtr<ID3D11RasterizerState> RenderStates::WireframeRS;
ComPtr<ID3D11RasterizerState> RenderStates::NoCullRS;
ComPtr<ID3D11RasterizerState> RenderStates::CullClockwiseRS;

ComPtr<ID3D11BlendState> RenderStates::AlphaToCoverageBS;
ComPtr<ID3D11BlendState> RenderStates::TransparentBS;
ComPtr<ID3D11BlendState> RenderStates::NoRenderTargetWritesBS;

ComPtr<ID3D11DepthStencilState> RenderStates::MarkMirrorDSS;
ComPtr<ID3D11DepthStencilState> RenderStates::DrawReflectionDSS;
ComPtr<ID3D11DepthStencilState> RenderStates::NoDoubleBlendDSS;

ComPtr<ID3D11SamplerState> RenderStates::AnisotropicSS;

void RenderStates::InitAll(ID3D11Device* device)
{
	// Wireframe Rasterizer State
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&wireframeDesc, WireframeRS.GetAddressOf()));

	// No Cull Rasterizer State
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&noCullDesc, NoCullRS.GetAddressOf()));

	// Cull clockwise rasterizer state
	// Note: Define such that we still cull backfaces by making front faces CCW.
	// If we did not cull backfaces, then we have to worry about the BackFace
	// property in the D3D11_DEPTH_STENCIL_DESC.
	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;
	ThrowIfFailed(device->CreateRasterizerState(&cullClockwiseDesc, CullClockwiseRS.GetAddressOf()));

	// Alpha to Coverage Blend State
	D3D11_BLEND_DESC alphaToCoverageDesc;
	ZeroMemory(&alphaToCoverageDesc, sizeof(D3D11_BLEND_DESC));
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	ThrowIfFailed(device->CreateBlendState(&alphaToCoverageDesc, AlphaToCoverageBS.GetAddressOf()));

	// Transparent Blend State
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
	
	// No render target writes blend state
	D3D11_BLEND_DESC noRenderTargetWritesDesc;
	ZeroMemory(&noRenderTargetWritesDesc, sizeof(D3D11_BLEND_DESC));
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

	// Mark mirror depth/stencil state
	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable = true;
	mirrorDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	mirrorDesc.DepthFunc = D3D11_COMPARISON_LESS;
	mirrorDesc.StencilEnable = true;
	mirrorDesc.StencilReadMask = 0xff;
	mirrorDesc.StencilWriteMask = 0xff;

	mirrorDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	mirrorDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	ThrowIfFailed(device->CreateDepthStencilState(&mirrorDesc, MarkMirrorDSS.GetAddressOf()));

	// Draw reflection depth/stencil state
	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable = true;
	drawReflectionDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawReflectionDesc.StencilEnable = true;
	drawReflectionDesc.StencilReadMask = 0xff;
	drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	drawReflectionDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ThrowIfFailed(device->CreateDepthStencilState(&drawReflectionDesc, DrawReflectionDSS.GetAddressOf()));

	// No double blend depth/stencil state
	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable = true;
	noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable = true;
	noDoubleBlendDesc.StencilReadMask = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	ThrowIfFailed(device->CreateDepthStencilState(&noDoubleBlendDesc, NoDoubleBlendDSS.GetAddressOf()));

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

	MarkMirrorDSS.Reset();
	DrawReflectionDSS.Reset();
	NoDoubleBlendDSS.Reset();

	AnisotropicSS.Reset();
}
