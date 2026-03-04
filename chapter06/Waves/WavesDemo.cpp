#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "Waves.h"

#include <d3dcompiler.h>

using namespace DirectX;
using namespace Microsoft::WRL;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class WavesDemo : public D3DApp
{
public:
	WavesDemo(HINSTANCE hInstance);
	~WavesDemo();

	bool Init();
	void UpdateScene(float dt);
	void DrawScene();

	void OnResize();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildLandGeometryBuffers();
	void BuildWavesGeometryBuffers();
	void BuildShaders();
	void BuildInputLayout();
	void BuildConstantBuffer();

	float GetHeight(float x, float z) const;

private:
	ComPtr<ID3D11Buffer> mLandVB;
	ComPtr<ID3D11Buffer> mLandIB;
	ComPtr<ID3D11Buffer> mWavesVB;
	ComPtr<ID3D11Buffer> mWavesIB;
	ComPtr<ID3D11Buffer> mCB;

	ComPtr<ID3D11InputLayout> mInputLayout;
	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;

	ComPtr<ID3D11RasterizerState> mWireframeRS;

	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mWavesWorld;

	UINT mGridIndexCount = 0;

	Waves mWaves;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta = 1.5f * MathHelper::Pi;
	float mPhi = 0.1f * MathHelper::Pi;
	float mRadius = 200.0f;

	POINT mLastMousePos = { 0, 0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	WavesDemo theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

WavesDemo::WavesDemo(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Waves Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

WavesDemo::~WavesDemo()
{
}

bool WavesDemo::Init()
{
	if (!D3DApp::Init()) {
		return false;
	}

	mWaves.Init(200, 200, 0.8f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWavesGeometryBuffers();
	BuildShaders();
	BuildInputLayout();
	BuildConstantBuffer();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	ThrowIfFailed(md3dDevice->CreateRasterizerState(&wireframeDesc, mWireframeRS.GetAddressOf()));

	return true;
}

void WavesDemo::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;
	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);
	mGridIndexCount = static_cast<UINT>(grid.Indices.size());

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i) {
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHeight(p.x, p.z);
		vertices[i].Pos = p;
		// Color the vertex based on its height.
		if (p.y < -10.0f)
		{
			// Sandy beach color.
			vertices[i].Color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (p.y < 5.0f)
		{
			// Light yellow-green.
			vertices[i].Color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (p.y < 12.0f)
		{
			// Dark yellow-green.
			vertices[i].Color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (p.y < 20.0f)
		{
			// Dark brown.
			vertices[i].Color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			// White snow.
			vertices[i].Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(grid.Vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, mLandVB.GetAddressOf()));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mGridIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mLandIB.GetAddressOf()));
}

float WavesDemo::GetHeight(float x, float z) const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}


void WavesDemo::BuildWavesGeometryBuffers()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, 0, mWavesVB.GetAddressOf()));

	std::vector<UINT> indices(3 * mWaves.TriangleCount()); // 3 indices per face

	// Iterate over each quad.
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (DWORD j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mWavesIB.GetAddressOf()));
}

void WavesDemo::BuildShaders()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errorMsg;

	// create vertex shader
	HRESULT hr = D3DCompileFromFile(
		L"color_vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr)) {
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return;
	}

	ThrowIfFailed(md3dDevice->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));

	// create pixel shader
	hr = D3DCompileFromFile(
		L"color_ps.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr)) {
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return;
	}

	ThrowIfFailed(md3dDevice->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));
}

void WavesDemo::BuildInputLayout()
{
	D3D11_INPUT_ELEMENT_DESC desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errorMsg;

	HRESULT hr = D3DCompileFromFile(
		L"color_vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr)) {
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return;
	}

	ThrowIfFailed(md3dDevice->CreateInputLayout(desc, 2, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), mInputLayout.GetAddressOf()));
}

void WavesDemo::BuildConstantBuffer()
{
	D3D11_BUFFER_DESC cbd;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(XMMATRIX);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;
	ThrowIfFailed(md3dDevice->CreateBuffer(&cbd, nullptr, mCB.GetAddressOf()));
}

void WavesDemo::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.25f) {
		t_base += 0.25f;

		DWORD i = 5 + rand() % 190;
		DWORD j = 5 + rand() % 190;

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	ThrowIfFailed(md3dImmediateContext->Map(mWavesVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i) {
		v[i].Pos = mWaves[i];
		v[i].Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	md3dImmediateContext->Unmap(mWavesVB.Get(), 0);
}

void WavesDemo::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout.Get());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	md3dImmediateContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	md3dImmediateContext->VSSetConstantBuffers(0, 1, mCB.GetAddressOf());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	// land
	md3dImmediateContext->IASetVertexBuffers(0, 1, mLandVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mLandIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
	XMMATRIX worldViewProj = XMMatrixTranspose(world * view * proj);
	D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
	md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &worldViewProj, sizeof(XMMATRIX));
	md3dImmediateContext->Unmap(mCB.Get(), 0);
	md3dImmediateContext->DrawIndexed(mGridIndexCount, 0, 0);

	// waves
	md3dImmediateContext->RSSetState(mWireframeRS.Get());

	md3dImmediateContext->IASetVertexBuffers(0, 1, mWavesVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mWavesIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	world = XMLoadFloat4x4(&mWavesWorld);
	worldViewProj = XMMatrixTranspose(world * view * proj);
	ZeroMemory(&mapped, sizeof(D3D11_MAPPED_SUBRESOURCE));
	md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	memcpy(mapped.pData, &worldViewProj, sizeof(XMMATRIX));
	md3dImmediateContext->Unmap(mCB.Get(), 0);
	md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

	md3dImmediateContext->RSSetState(nullptr);

	ThrowIfFailed(mSwapChain->Present(0, 0));
}

void WavesDemo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void WavesDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void WavesDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void WavesDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
