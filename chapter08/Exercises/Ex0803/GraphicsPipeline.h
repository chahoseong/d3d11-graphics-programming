#pragma once

#include "LightHelper.h"

#include <array>
#include <cstring>
#include <string>
#include <type_traits>

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

struct ShaderBytecodeView
{
	const void* pointer = nullptr;
	SIZE_T size = 0;
};

template <typename T>
class ConstantBuffer
{
public:
	T data;
	Microsoft::WRL::ComPtr<ID3D11Buffer> handle;

	ConstantBuffer(ID3D11Device* device)
	{
		static_assert(std::is_trivially_copyable_v<T>, "Constant buffers must be trivially copyable.");
		static_assert((sizeof(T) % 16) == 0, "Constant buffer size must be a multiple of 16 bytes.");

		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = sizeof(T);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		device->CreateBuffer(&desc, nullptr, handle.GetAddressOf());
	}

	void Update(ID3D11DeviceContext* context) const
	{
		D3D11_MAPPED_SUBRESOURCE mapped;
		context->Map(handle.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		memcpy(mapped.pData, &data, sizeof(T));
		context->Unmap(handle.Get(), 0);
	}
};

class GraphicsPipeline
{
public:
	GraphicsPipeline(ID3D11Device* device);

protected:
	Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const std::string& target) const;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> CreateVertexShader(const std::wstring& filename, Microsoft::WRL::ComPtr<ID3DBlob>* outCompiledShader);
	Microsoft::WRL::ComPtr<ID3D11PixelShader> CreatePixelShader(const std::wstring& filename, Microsoft::WRL::ComPtr<ID3DBlob>* outCompiledShader);

protected:
	ID3D11Device* md3dDevice;
};

// BasicGraphicsPipeline owns the shader pair and constant buffers for the
// chapter 7 directional-lighting example.
class BasicGraphicsPipeline : public GraphicsPipeline
{
	struct alignas(16) FrameData
	{
		std::array<DirectionalLight, 3> dirLights;
		int lightCount;
		DirectX::XMFLOAT3 eyePosW;

		float fogStart;
		float fogRange;
		DirectX::XMFLOAT4 fogColor;
	};

	struct alignas(16) ObjectData
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 worldInvTranspose;
		DirectX::XMFLOAT4X4 worldViewProj;
		DirectX::XMFLOAT4X4 texTransform;
		Material material;
		int useTexture;
	};

public:
	enum ShaderSlot : UINT
	{
		FrameBufferSlot = 0,
		ObjectBufferSlot = 1,
	};

	BasicGraphicsPipeline(ID3D11Device* device);

	void SetWorld(DirectX::CXMMATRIX M);
	void SetWorldInvTranspose(DirectX::CXMMATRIX M);
	void SetWorldViewProj(DirectX::CXMMATRIX M);
	void SetTexTransform(DirectX::CXMMATRIX M);
	void SetEyePosW(const DirectX::XMFLOAT3& v);
	void SetDirLights(const DirectionalLight* lights, int count);
	void SetMaterial(const Material& mat);
	void SetDiffuseMap(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture);
	void SetFlareDiffuseMap(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture);
	void SetFlareAlphaMap(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture);
	void SetUseTexture(bool enabled);
	bool GetUseTexture() const;

	void SetSamplerState(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void Apply(ID3D11DeviceContext* context) const;
	ShaderBytecodeView GetVertexShaderBytecode() const;

private:
	void LoadShaders();
	void UpdateFrameBuffer(ID3D11DeviceContext* context) const;
	void UpdateObjectBuffer(ID3D11DeviceContext* context) const;
	void Bind(ID3D11DeviceContext* context) const;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> mCompiledVertexShader;

	ConstantBuffer<FrameData> mFrameCB;
	ConstantBuffer<ObjectData> mObjectCB;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mDiffuseMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mFlareDiffuseMap;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mFlareAlphaMap;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> mSamplerState;
};

class Pipelines
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicGraphicsPipeline* Basic;
};
