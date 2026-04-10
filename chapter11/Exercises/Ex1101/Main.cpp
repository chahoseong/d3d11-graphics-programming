#include "d3dApp.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Vertex.h"
#include "GraphicsPipeline.h"
#include "RenderStates.h"

using namespace DirectX;
using namespace Microsoft::WRL;

class CylinderApp : public D3DApp
{
public:
	CylinderApp(HINSTANCE hInstance);
	~CylinderApp();

	bool Init() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildCylinderGeometryBuffer();

private:
	ComPtr<ID3D11Buffer> mCylinderVB;

	DirectionalLight mDirLights[3];
	Material mCylinderMat;

	XMFLOAT4X4 mCylinderWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT3 mEyePosW;

	float mTheta = 1.3f * MathHelper::Pi;
	float mPhi = 0.4f * MathHelper::Pi;
	float mRadius = 120.0f;

	POINT mLastMousePos = { 0, 0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CylinderApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

CylinderApp::CylinderApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Cylinder Demo";
	mEnable4xMsaa = true;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mCylinderWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mCylinderMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mCylinderMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mCylinderMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

CylinderApp::~CylinderApp()
{
	Pipelines::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool CylinderApp::Init()
{
	if (!D3DApp::Init()) {
		return false;
	}

	Pipelines::InitAll(md3dDevice.Get());
	InputLayouts::InitAll(md3dDevice.Get());
	RenderStates::InitAll(md3dDevice.Get());

	BuildCylinderGeometryBuffer();

	return true;
}

void CylinderApp::BuildCylinderGeometryBuffer()
{
	Vertex::Cylinder v;
	v.Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	v.Size = XMFLOAT2(50, 200);

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(Vertex::Cylinder);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &v;
	ThrowIfFailed(md3dDevice->CreateBuffer(&desc, &vinitData, mCylinderVB.GetAddressOf()));
}

void CylinderApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void CylinderApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(InputLayouts::Cylinder.Get());

	UINT stride = sizeof(Vertex::Cylinder);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, mCylinderVB.GetAddressOf(), &stride, &offset);

	XMMATRIX world = XMLoadFloat4x4(&mCylinderWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world * view * proj;

	Pipelines::Cylinder->SetWorld(world);
	Pipelines::Cylinder->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Cylinder->SetWorldViewProj(worldViewProj);
	Pipelines::Cylinder->SetEyePosW(mEyePosW);
	Pipelines::Cylinder->SetDirLights(mDirLights, 3);
	Pipelines::Cylinder->SetFogColor(Colors::Silver);
	Pipelines::Cylinder->SetFogStart(15.0f);
	Pipelines::Cylinder->SetFogRange(175.0f);

	Pipelines::Cylinder->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->Draw(1, 0);

	ThrowIfFailed(mSwapChain->Present(0, 0));
}

void CylinderApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CylinderApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CylinderApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CylinderApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		float dx = 0.1f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
