#pragma once

#include <d3d11.h>
#include <wrl.h>

class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11RasterizerState> WireframeRS;
	static Microsoft::WRL::ComPtr<ID3D11RasterizerState> NoCullRS;
	static Microsoft::WRL::ComPtr<ID3D11RasterizerState> CullClockwiseRS;

	static Microsoft::WRL::ComPtr<ID3D11BlendState> AlphaToCoverageBS;
	static Microsoft::WRL::ComPtr<ID3D11BlendState> TransparentBS;
	static Microsoft::WRL::ComPtr<ID3D11BlendState> NoRenderTargetWritesBS;

	static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> MarkMirrorDSS;
	static Microsoft::WRL::ComPtr<ID3D10DepthStencilState> DrawReflectionDSS;
	static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> NoDoubleBlendDSS;

	static Microsoft::WRL::ComPtr<ID3D11SamplerState> AnisotropicSS;
};