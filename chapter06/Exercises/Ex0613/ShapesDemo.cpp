#include "d3dapp.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"

#include <d3dcompiler.h>

using namespace DirectX;
using namespace Microsoft::WRL;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class ShapesApp : public D3DApp
{
public:
	ShapesApp(HINSTANCE hInstance);
	~ShapesApp();

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
	void BuildInputLayout();
	void BuildConstantBuffer();

private:
	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;
	ComPtr<ID3D11Buffer> mCB;

	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	ComPtr<ID3D11InputLayout> mInputLayout;

	ComPtr<ID3D11RasterizerState> mWireframeRS;

	XMFLOAT4X4 mSphereWorld[10];
	XMFLOAT4X4 mCylWorld[10];
	XMFLOAT4X4 mBoxWorld;
	XMFLOAT4X4 mGridWorld;
	XMFLOAT4X4 mCenterSphere;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset = 0;
	int mGridVertexOffset = 0;
	int mSphereVertexOffset = 0;
	int mCylinderVertexOffset = 0;

	UINT mBoxIndexOffset = 0;
	UINT mGridIndexOffset = 0;
	UINT mSphereIndexOffset = 0;
	UINT mCylinderIndexOffset = 0;

	UINT mBoxIndexCount = 0;
	UINT mGridIndexCount = 0;
	UINT mSphereIndexCount = 0;
	UINT mCylinderIndexCount = 0;

	float mTheta = 1.5f * MathHelper::Pi;
	float mPhi = 0.1f * MathHelper::Pi;
	float mRadius = 15.0f;

	POINT mLastMousePos = { 0, 0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	ShapesApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

ShapesApp::ShapesApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Shapes Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGridWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX boxScale = XMMatrixScaling(2.0f, 1.0f, 2.0f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMStoreFloat4x4(&mBoxWorld, XMMatrixMultiply(boxScale, boxOffset));

	XMMATRIX centerSphereScale = XMMatrixScaling(2.0f, 2.0f, 2.0f);
	XMMATRIX centerSphereOffset = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&mCenterSphere, XMMatrixMultiply(centerSphereScale, centerSphereOffset));

	for (int i = 0; i < 5; ++i) {
		XMStoreFloat4x4(&mCylWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 1.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mCylWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 1.5f, -10.0f + i * 5.0f));

		XMStoreFloat4x4(&mSphereWorld[i * 2 + 0], XMMatrixTranslation(-5.0f, 3.5f, -10.0f + i * 5.0f));
		XMStoreFloat4x4(&mSphereWorld[i * 2 + 1], XMMatrixTranslation(+5.0f, 3.5f, -10.0f + i * 5.0f));

	}
}

ShapesApp::~ShapesApp()
{
}

bool ShapesApp::Init()
{
	if (!D3DApp::Init()) {
		return false;
	}

	BuildGeometryBuffers();
	BuildShaders();
	BuildInputLayout();
	BuildConstantBuffer();

	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	// wireframeDesc.ScissorEnable = true;
	ThrowIfFailed(md3dDevice->CreateRasterizerState(&wireframeDesc, mWireframeRS.GetAddressOf()));

	return true;
}

void ShapesApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData grid;
	GeometryGenerator::MeshData sphere;
	GeometryGenerator::MeshData cylinder;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(20.0f, 30.0f, 60, 40, grid);
	geoGen.CreateSphere(0.5f, 20, 20, sphere);
	// geoGen.CreateGeosphere(0.5f, 2, sphere);
	geoGen.CreateCylinder(0.5, 0.3f, 3.0f, 20, 20, cylinder);

	mBoxVertexOffset = 0;
	mGridVertexOffset = static_cast<UINT>(box.Vertices.size());
	mSphereVertexOffset = mGridVertexOffset + static_cast<UINT>(grid.Vertices.size());
	mCylinderVertexOffset = mSphereVertexOffset + static_cast<UINT>(sphere.Vertices.size());

	mBoxIndexCount = static_cast<UINT>(box.Indices.size());
	mGridIndexCount = static_cast<UINT>(grid.Indices.size());
	mSphereIndexCount = static_cast<UINT>(sphere.Indices.size());
	mCylinderIndexCount = static_cast<UINT>(cylinder.Indices.size());

	mBoxIndexOffset = 0;
	mGridIndexOffset = mBoxIndexCount;
	mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
	mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	UINT totalVertexCount = static_cast<UINT>(
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size()
	);

	UINT totalIndexCount =
		mBoxIndexCount +
		mGridIndexCount +
		mSphereIndexCount +
		mCylinderIndexCount;

	//
	std::vector<Vertex> vertices(totalVertexCount);
	
	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);

	UINT k = 0;
	
	// box
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k) {
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = black;
	}

	// grid
	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k) {
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = black;
	}

	// sphere
	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k) {
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = black;
	}

	// cylinder
	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k) {
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = black;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, mVB.GetAddressOf()));

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());
	indices.insert(indices.end(), grid.Indices.begin(), grid.Indices.end());
	indices.insert(indices.end(), sphere.Indices.begin(), sphere.Indices.end());
	indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mIB.GetAddressOf()));
}

void ShapesApp::BuildShaders()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
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

void ShapesApp::BuildInputLayout()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ThrowIfFailed(md3dDevice->CreateInputLayout(vertexDesc, 2, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), mInputLayout.GetAddressOf()));
}

void ShapesApp::BuildConstantBuffer()
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

void ShapesApp::UpdateScene(float dt)
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
}

void ShapesApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout.Get());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->RSSetState(mWireframeRS.Get());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	md3dImmediateContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	md3dImmediateContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	md3dImmediateContext->VSSetConstantBuffers(0, 1, mCB.GetAddressOf());

	//D3D11_RECT rects = { 100, 100, 400, 400 };
	//md3dImmediateContext->RSSetScissorRects(1, &rects);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// grid
	XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	XMMATRIX wvp = XMMatrixTranspose(world * view * proj);
	memcpy(mapped.pData, &wvp, sizeof(XMMATRIX));
	md3dImmediateContext->Unmap(mCB.Get(), 0);
	md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

	// box
	world = XMLoadFloat4x4(&mBoxWorld);
	ZeroMemory(&mapped, sizeof(mapped));
	md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	wvp = XMMatrixTranspose(world * view * proj);
	memcpy(mapped.pData, &wvp, sizeof(XMMATRIX));
	md3dImmediateContext->Unmap(mCB.Get(), 0);
	md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	// center sphere
	world = XMLoadFloat4x4(&mCenterSphere);
	ZeroMemory(&mapped, sizeof(mapped));
	md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	wvp = XMMatrixTranspose(world * view * proj);
	memcpy(mapped.pData, &wvp, sizeof(XMMATRIX));
	md3dImmediateContext->Unmap(mCB.Get(), 0);
	md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

	// cylinders
	for (int i = 0; i < 10; ++i) {
		world = XMLoadFloat4x4(&mCylWorld[i]);
		ZeroMemory(&mapped, sizeof(mapped));
		md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		wvp = XMMatrixTranspose(world * view * proj);
		memcpy(mapped.pData, &wvp, sizeof(XMMATRIX));
		md3dImmediateContext->Unmap(mCB.Get(), 0);
		md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
	}

	// spheres
	for (int i = 0; i < 10; ++i) {
		world = XMLoadFloat4x4(&mSphereWorld[i]);
		ZeroMemory(&mapped, sizeof(mapped));
		md3dImmediateContext->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
		wvp = XMMatrixTranspose(world * view * proj);
		memcpy(mapped.pData, &wvp, sizeof(XMMATRIX));
		md3dImmediateContext->Unmap(mCB.Get(), 0);
		md3dImmediateContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);
	}

	ThrowIfFailed(mSwapChain->Present(0, 0));
}

void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 200.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
