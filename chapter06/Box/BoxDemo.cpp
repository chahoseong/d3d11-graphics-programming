#include "d3dapp.h"
#include "MathHelper.h"

#include <d3dcompiler.h>


using namespace DirectX;
using namespace Microsoft::WRL;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	~BoxApp();

	bool Init() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;
	
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildGeometryBuffers();
	void BuildShaders();
	void BuildVertexLayout();

private:
	ComPtr<ID3D11Buffer> mBoxVB;
	ComPtr<ID3D11Buffer> mBoxIB;

	ComPtr<ID3D11InputLayout> mInputLayout;
	ComPtr<ID3D11Buffer> mConstantBuffer;
	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta = 1.5f * MathHelper::Pi;
	float mPhi = 0.25f * MathHelper::Pi;
	float mRadius = 5.0f;

	POINT mLastMousePos = { 0, 0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BoxApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

BoxApp::BoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Box Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

BoxApp::~BoxApp()
{
}

bool BoxApp::Init()
{
	if (!D3DApp::Init()) {
		return false;
	}

	BuildGeometryBuffers();
	BuildShaders();
	BuildVertexLayout();

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(XMFLOAT4X4);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	ThrowIfFailed(md3dDevice->CreateBuffer(&desc, nullptr, mConstantBuffer.GetAddressOf()));

	return true;
}

void BoxApp::BuildGeometryBuffers()
{
	Vertex vertices[] = {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), Convert::ToXmFloat4(Colors::White) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), Convert::ToXmFloat4(Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), Convert::ToXmFloat4(Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), Convert::ToXmFloat4(Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), Convert::ToXmFloat4(Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), Convert::ToXmFloat4(Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), Convert::ToXmFloat4(Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), Convert::ToXmFloat4(Colors::Magenta) }
	};

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, mBoxVB.GetAddressOf()));

	UINT indices[] = {
		// front
		0, 1, 2,
		0, 2, 3,

		// back
		4, 6, 5,
		4, 7, 6,

		// left
		4, 5, 1,
		4, 1, 0,

		// right
		3, 2, 6,
		3, 6, 7,

		// top
		1, 5, 6,
		1, 6, 2,

		// bottom
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mBoxIB.GetAddressOf()));
}

void BoxApp::BuildShaders()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> error;
	
	// compile and create vertex shader
	HRESULT hr = D3DCompileFromFile(
		L"color_vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		error.GetAddressOf()
	);
	
	if (FAILED(hr))
	{
		const std::wstring msg = TextHelper::ToString(reinterpret_cast<char*>(error->GetBufferPointer()));
		MessageBox(NULL, msg.c_str(), NULL, MB_OK);
		return;
	}

	ThrowIfFailed(md3dDevice->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));

	// compile and create pixel shader
	hr = D3DCompileFromFile(
		L"color_ps.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		error.GetAddressOf()
	);

	if (FAILED(hr))
	{
		if (error) {
			const std::wstring msg = TextHelper::ToString(reinterpret_cast<char*>(error->GetBufferPointer()));
			MessageBox(NULL, msg.c_str(), NULL, MB_OK);
		}
		return;
	}

	ThrowIfFailed(md3dDevice->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));
}

void BoxApp::BuildVertexLayout()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> error;

	HRESULT hr = D3DCompileFromFile(
		L"color_vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		error.GetAddressOf()
	);

	if (FAILED(hr))
	{
		if (error) {
			const std::wstring msg = TextHelper::ToString(reinterpret_cast<char*>(error->GetBufferPointer()));
			MessageBox(NULL, msg.c_str(), NULL, MB_OK);
		}
		return;
	}

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ThrowIfFailed(md3dDevice->CreateInputLayout(vertexDesc, 2, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), mInputLayout.GetAddressOf()));
}

void BoxApp::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void BoxApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), Colors::LightSteelBlue);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout.Get());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, mBoxVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	// direct3d는 행 벡터, hlsl은 열 벡터를 사용하므로 hlsl에 값을
	// 제대로 전달하려면 전치를 해야합니다.
	XMMATRIX worldViewProj = XMMatrixTranspose(world * view * proj);

	// update constant buffer
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	md3dImmediateContext->Map(mConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &worldViewProj, sizeof(XMMATRIX));
	md3dImmediateContext->Unmap(mConstantBuffer.Get(), 0);

	md3dImmediateContext->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());

	// set shaders
	md3dImmediateContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	md3dImmediateContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	// draw model
	md3dImmediateContext->DrawIndexed(36, 0, 0);

	// present front buffer
	ThrowIfFailed(mSwapChain->Present(0, 0));
}

void BoxApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0) {
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mTheta += dx;
		mPhi += dy;

		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0) {
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		mRadius += dx - dy;

		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
