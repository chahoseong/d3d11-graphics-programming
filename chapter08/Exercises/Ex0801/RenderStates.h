#pragma once

#include <d3d11.h>
#include <wrl.h>

class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static Microsoft::WRL::ComPtr<ID3D11SamplerState> AnisotropicSS;
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> LinearSS;
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> PointSS;
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> WarpSS;
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> ClampSS;
	static Microsoft::WRL::ComPtr<ID3D11SamplerState> MirrorSS;
};