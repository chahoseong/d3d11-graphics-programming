#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Waves.h"

#include "d3dcompiler.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

using namespace DirectX;
using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct FrameBuffer
{
	DirectionalLight DirLight;
	PointLight PointLight;
	SpotLight SpotLight;
	XMFLOAT3 EyePos;
	float pad;
};

struct ObjectBuffer
{
	XMFLOAT4X4 world;
	XMFLOAT4X4 worldInvTranspose;
	XMFLOAT4X4 worldViewProj;
	Material material;
};

class LightingApp : public D3DApp
{
public:
	LightingApp(HINSTANCE hInstance);
	~LightingApp();

	bool Init() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnResize() override;
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM bntState, int x, int y) override;

private:
	bool InitImGui();
	void ShutdownImGui();
	void DrawImGui();

	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildShaders();
	void BuildInputLayout();
	void BuildConstantBuffers();

	float GetHillHeight(float x, float z) const;
	XMFLOAT3 GetHillNormal(float x, float z) const;

private:
	ComPtr<ID3D11Buffer> mLandVB;
	ComPtr<ID3D11Buffer> mLandIB;
	ComPtr<ID3D11Buffer> mWavesVB;
	ComPtr<ID3D11Buffer> mWavesIB;

	ComPtr<ID3D11Buffer> mFrameCB;
	ComPtr<ID3D11Buffer> mObjectCB;

	Waves mWaves;

	DirectionalLight mDirLight;
	PointLight mPointLight;
	SpotLight mSpotLight;

	Material mLandMat;
	Material mWavesMat;

	ComPtr<ID3D11VertexShader> mVertexShader;
	ComPtr<ID3D11PixelShader> mPixelShader;
	ComPtr<ID3D11InputLayout> mInputLayout;

	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mLandIndexCount = 0;

	XMFLOAT3 mEyePosW;

	float mTheta = 1.5f * MathHelper::Pi;
	float mPhi = 0.1f * MathHelper::Pi;
	float mRadius = 80.0f;

	POINT mLastMousePos = { 0, 0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	LightingApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

LightingApp::LightingApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Lighting Demo";

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX wavesOffset = XMMatrixTranslation(0.0f, -3.0f, 0.0f);
	XMStoreFloat4x4(&mWavesWorld, wavesOffset);

	// Directional light
	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	// Point light
	mPointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Att = XMFLOAT3(0.0f, 1.0f, 0.0f);
	mPointLight.Range = 25.0f;

	// Spot light
	mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot = 96.0f;
	mSpotLight.Range = 10000.0f;

	mLandMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(0.48f, 0.77, 0.46f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(0.137f, 0.42f, 0.556f, 1.0f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 96.0f);
}

LightingApp::~LightingApp()
{
	ShutdownImGui();
}

bool LightingApp::Init()
{
	if (!D3DApp::Init()) {
		return false;
	}

	mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildShaders();
	BuildInputLayout();
	BuildConstantBuffers();

	return InitImGui();
}

bool LightingApp::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	if (!ImGui_ImplWin32_Init(mhMainWnd)) {
		ImGui::DestroyContext();
		return false;
	}

	if (!ImGui_ImplDX11_Init(md3dDevice.Get(), md3dImmediateContext.Get())) {
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		return false;
	}

	return true;
}

void LightingApp::ShutdownImGui()
{
	if (ImGui::GetCurrentContext()) {
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}

void LightingApp::DrawImGui()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	float spot = mSpotLight.Spot;

	ImGui::Begin("Lighting Controls");
	ImGui::Text("Orbit: Left mouse button");
	ImGui::Text("Zoom: Right mouse button");
	ImGui::Text("Camera radius: %.2f", mRadius);
	ImGui::Text("Eye position: (%.2f, %.2f, %.2f)", mEyePosW.x, mEyePosW.y, mEyePosW.z);

	if (ImGui::InputFloat("Spot", &spot, 1.0f)) {
		mSpotLight.Spot = spot;
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void LightingApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;
	geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

	mLandIndexCount = grid.Indices.size();

	std::vector<Vertex> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i) {
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHillHeight(p.x, p.z);
		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * static_cast<UINT>(vertices.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, mLandVB.GetAddressOf()));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &grid.Indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mLandIB.GetAddressOf()));
}

float LightingApp::GetHillHeight(float x, float z)const
{
	return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
}

XMFLOAT3 LightingApp::GetHillNormal(float x, float z) const
{
	XMFLOAT3 n(
		-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
		1.0f,
		-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z)
	);

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void LightingApp::BuildWaveGeometryBuffers()
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(Vertex) * mWaves.VertexCount();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, nullptr, mWavesVB.GetAddressOf()));

	std::vector<UINT> indices(3 * mWaves.TriangleCount());
	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();
	int k = 0;
	for (UINT i = 0; i < m - 1; ++i) {
		for (DWORD j = 0; j < n - 1; ++j) {
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;

			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6;
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mWavesIB.GetAddressOf()));
}


void LightingApp::BuildShaders()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errorMsg;

	// compile and create vertex shader
	HRESULT hr = D3DCompileFromFile(
		L"lighting_vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr))
	{
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return;
	}

	ThrowIfFailed(md3dDevice->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, mVertexShader.GetAddressOf()));

	// compile and create pixel shader
	hr = D3DCompileFromFile(
		L"lighting_ps.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr))
	{
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return;
	}

	ThrowIfFailed(md3dDevice->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), nullptr, mPixelShader.GetAddressOf()));
}

void LightingApp::BuildInputLayout()
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> compiledShader;
	ComPtr<ID3DBlob> errorMsg;

	HRESULT hr = D3DCompileFromFile(
		L"lighting_vs.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		flags,
		0,
		compiledShader.GetAddressOf(),
		errorMsg.GetAddressOf()
	);

	if (FAILED(hr))
	{
		if (errorMsg) {
			std::cerr << errorMsg->GetBufferPointer() << std::endl;
		}
		return;
	}

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ThrowIfFailed(md3dDevice->CreateInputLayout(vertexDesc, 2, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), mInputLayout.GetAddressOf()));
}

void LightingApp::BuildConstantBuffers()
{
	D3D11_BUFFER_DESC frameDesc;
	frameDesc.Usage = D3D11_USAGE_DYNAMIC;
	frameDesc.ByteWidth = sizeof(FrameBuffer);
	frameDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	frameDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	frameDesc.MiscFlags = 0;
	frameDesc.StructureByteStride = 0;
	ThrowIfFailed(md3dDevice->CreateBuffer(&frameDesc, nullptr, mFrameCB.GetAddressOf()));

	D3D11_BUFFER_DESC objectDesc;
	objectDesc.Usage = D3D11_USAGE_DYNAMIC;
	objectDesc.ByteWidth = sizeof(ObjectBuffer);
	objectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	objectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	objectDesc.MiscFlags = 0;
	objectDesc.StructureByteStride = 0;
	ThrowIfFailed(md3dDevice->CreateBuffer(&objectDesc, nullptr, mObjectCB.GetAddressOf()));
}

void LightingApp::UpdateScene(float dt)
{
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	static float t_base = 0.0f;
	if ((mTimer.TotalTime() - t_base) >= 0.25f) {
		t_base += 0.25f;

		DWORD i = 5 + rand() % (mWaves.RowCount() - 10);
		DWORD j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves.Disturb(i, j, r);
	}

	mWaves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	ThrowIfFailed(md3dImmediateContext->Map(mWavesVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

	Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i) {
		v[i].Pos = mWaves[i];
		v[i].Normal = mWaves.Normal(i);
	}

	md3dImmediateContext->Unmap(mWavesVB.Get(), 0);

	mPointLight.Position.x = 70.0f * cosf(0.2f * mTimer.TotalTime());
	mPointLight.Position.z = 70.0f * sinf(0.2f * mTimer.TotalTime());
	mPointLight.Position.y = MathHelper::Max(GetHillHeight(mPointLight.Position.x, mPointLight.Position.z), -3.0f) + 10.0f;

	mSpotLight.Position = mEyePosW;
	XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));
}

void LightingApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(mInputLayout.Get());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dImmediateContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
	md3dImmediateContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

	md3dImmediateContext->VSSetConstantBuffers(1, 1, mObjectCB.GetAddressOf());

	ID3D11Buffer* ps_constantbuffers[] = { mFrameCB.Get(), mObjectCB.Get() };
	md3dImmediateContext->PSSetConstantBuffers(0, 2, ps_constantbuffers);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	FrameBuffer fb = { mDirLight, mPointLight, mSpotLight, mEyePosW };
	D3D11_MAPPED_SUBRESOURCE mappedData;
	md3dImmediateContext->Map(mFrameCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy(mappedData.pData, &fb, sizeof(FrameBuffer));
	md3dImmediateContext->Unmap(mFrameCB.Get(), 0);

	// Draw the hills.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mLandVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mLandIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;
	ObjectBuffer ob;
	XMStoreFloat4x4(&ob.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&ob.worldInvTranspose, XMMatrixTranspose(worldInvTranspose));
	XMStoreFloat4x4(&ob.worldViewProj, XMMatrixTranspose(worldViewProj));
	ob.material = mLandMat;
	ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));
	md3dImmediateContext->Map(mObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy(mappedData.pData, &ob, sizeof(ObjectBuffer));
	md3dImmediateContext->Unmap(mObjectCB.Get(), 0);

	md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);

	// Draw the waves.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mWavesVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mWavesIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	world = XMLoadFloat4x4(&mWavesWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world * view * proj;

	ZeroMemory(&ob, sizeof(ObjectBuffer));
	XMStoreFloat4x4(&ob.world, XMMatrixTranspose(world));
	XMStoreFloat4x4(&ob.worldInvTranspose, XMMatrixTranspose(worldInvTranspose));
	XMStoreFloat4x4(&ob.worldViewProj, XMMatrixTranspose(worldViewProj));
	ob.material = mWavesMat;
	ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));
	md3dImmediateContext->Map(mObjectCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	memcpy(mappedData.pData, &ob, sizeof(ObjectBuffer));
	md3dImmediateContext->Unmap(mObjectCB.Get(), 0);

	md3dImmediateContext->DrawIndexed(mWaves.TriangleCount() * 3, 0, 0);

	DrawImGui();

	ThrowIfFailed(mSwapChain->Present(0, 0));
}

void LightingApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

LRESULT LightingApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam)) {
		return true;
	}

	return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
}

void LightingApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse) {
		return;
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mhMainWnd);
}

void LightingApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void LightingApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if (ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse) {
		mLastMousePos.x = x;
		mLastMousePos.y = y;
		return;
	}

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
