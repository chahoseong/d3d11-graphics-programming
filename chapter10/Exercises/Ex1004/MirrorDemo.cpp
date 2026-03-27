#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "GraphicsPipeline.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "Waves.h"

#include "directxtk/DDSTextureLoader.h"

using namespace DirectX;
using namespace Microsoft::WRL;

enum class RenderOptions
{
	Lighting,
	Textures,
	TexturesAndFog,
};

class MirrorApp : public D3DApp
{
public:
	MirrorApp(HINSTANCE hInstance);
	~MirrorApp();

	bool Init() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;
	
	void OnResize() override;

	void OnMouseDown(WPARAM btnState, int x, int y) override;
	void OnMouseUp(WPARAM btnState, int x, int y) override;
	void OnMouseMove(WPARAM btnState, int x, int y) override;

private:
	void BuildRoomGeometryBuffers();
	void BuildSkullGeometryBuffers();

private:
	ComPtr<ID3D11Buffer> mRoomVB;
	ComPtr<ID3D11Buffer> mSkullVB;
	ComPtr<ID3D11Buffer> mSkullIB;

	ComPtr<ID3D11ShaderResourceView> mFloorDiffuseMapSRV;
	ComPtr<ID3D11ShaderResourceView> mWallDiffuseMapSRV;
	ComPtr<ID3D11ShaderResourceView> mMirrorDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	Material mRoomMat;
	Material mSkullMat;
	Material mMirrorMat;
	Material mShadowMat;

	XMFLOAT4X4 mRoomWorld;
	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;
	XMFLOAT3 mSkullTranslation;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	RenderOptions mRenderOptions = RenderOptions::Textures;

	XMFLOAT3 mEyePosW;

	float mTheta = 1.24f * MathHelper::Pi;
	float mPhi = 0.42f * MathHelper::Pi;
	float mRadius = 12.0f;

	POINT mLastMousePos = { 0, 0 };
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	MirrorApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

MirrorApp::MirrorApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mSkullTranslation(0.0f, 1.0f, -5.0f)
{
	mMainWndCaption = L"Mirror Demo";
	mEnable4xMsaa = false;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mRoomWorld, I);
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

	mRoomMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mRoomMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mRoomMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	// Reflected material is transparent so it blends into mirror.
	mMirrorMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mMirrorMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mMirrorMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);

	mShadowMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mShadowMat.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	mShadowMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
}

MirrorApp::~MirrorApp()
{
	md3dImmediateContext->ClearState();
	Pipelines::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool MirrorApp::Init()
{
	if (!D3DApp::Init()) {
		return false;
	}

	Pipelines::InitAll(md3dDevice.Get());
	InputLayouts::InitAll(md3dDevice.Get(), Pipelines::Basic->GetVertexShaderBytecode());
	RenderStates::InitAll(md3dDevice.Get());

	CreateDDSTextureFromFile(
		md3dDevice.Get(),
		L"Textures/checkboard.dds",
		nullptr,
		mFloorDiffuseMapSRV.GetAddressOf()
	);

	CreateDDSTextureFromFile(
		md3dDevice.Get(),
		L"Textures/brick01.dds",
		nullptr,
		mWallDiffuseMapSRV.GetAddressOf()
	);

	CreateDDSTextureFromFile(
		md3dDevice.Get(),
		L"Textures/ice.dds",
		nullptr,
		mMirrorDiffuseMapSRV.GetAddressOf()
	);

	BuildRoomGeometryBuffers();
	BuildSkullGeometryBuffers();

	return true;
}

void MirrorApp::BuildRoomGeometryBuffers()
{
	// Create and specify geometry.  For this sample we draw a floor
	// and a wall with a mirror on it.  We put the floor, wall, and
	// mirror geometry in one vertex buffer.
	//
	//   |--------------|
	//   |              |
	//   |----|----|----|
	//   |Wall|Mirr|Wall|
	//   |    | or |    |
	//   /--------------/
	//  /   Floor      /
	// /--------------/

	std::vector<Vertex::Basic32> v(30);
	
	// Floor: Observe we tile texture coordinates.
	v[0] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[1] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
	v[2] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);

	v[3] = Vertex::Basic32(-3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f);
	v[4] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f);
	v[5] = Vertex::Basic32(7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f);

	// Wall: Observe we tile texture coordinates, and that we
	// leave a gap in the middle for the mirror.
	v[6] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[7] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[8] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);

	v[9] = Vertex::Basic32(-3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[10] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f);
	v[11] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f);

	v[12] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[13] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[14] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);

	v[15] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f);
	v[16] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f);
	v[17] = Vertex::Basic32(7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f);

	v[18] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[19] = Vertex::Basic32(-3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[20] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);

	v[21] = Vertex::Basic32(-3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[22] = Vertex::Basic32(7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f);
	v[23] = Vertex::Basic32(7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f);
	
	// Mirror
	v[24] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[25] = Vertex::Basic32(-2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[26] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);

	v[27] = Vertex::Basic32(-2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[28] = Vertex::Basic32(2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[29] = Vertex::Basic32(2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * static_cast<UINT>(v.size());
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &v[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, mRoomVB.GetAddressOf()));
}

void MirrorApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		std::cerr << "Models/skull.txt not found." << std::endl;
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	std::vector<Vertex::Basic32> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&vbd, &vinitData, mSkullVB.GetAddressOf()));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	ThrowIfFailed(md3dDevice->CreateBuffer(&ibd, &iinitData, mSkullIB.GetAddressOf()));
}

void MirrorApp::UpdateScene(float dt)
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

	//
	// Switch the render mode based in key input.
	//
	if (GetAsyncKeyState('1') & 0x8000)
		mRenderOptions = RenderOptions::Lighting;

	if (GetAsyncKeyState('2') & 0x8000)
		mRenderOptions = RenderOptions::Textures;

	if (GetAsyncKeyState('3') & 0x8000)
		mRenderOptions = RenderOptions::TexturesAndFog;

	//
	// Allow user to move box.
	//
	if (GetAsyncKeyState('A') & 0x8000)
		mSkullTranslation.x -= 1.0f * dt;

	if (GetAsyncKeyState('D') & 0x8000)
		mSkullTranslation.x += 1.0f * dt;

	if (GetAsyncKeyState('W') & 0x8000)
		mSkullTranslation.y += 1.0f * dt;

	if (GetAsyncKeyState('S') & 0x8000)
		mSkullTranslation.y -= 1.0f * dt;

	// Don't let user move below ground plane.
	mSkullTranslation.y = MathHelper::Max(mSkullTranslation.y, 0.0f);

	// Update the new world matrix.
	XMMATRIX skullRotate = XMMatrixRotationY(0.5f * MathHelper::Pi);
	XMMATRIX skullScale = XMMatrixScaling(0.45f, 0.45f, 0.45f);
	XMMATRIX skullOffset = XMMatrixTranslation(mSkullTranslation.x, mSkullTranslation.y, mSkullTranslation.z);
	XMStoreFloat4x4(&mSkullWorld, skullRotate * skullScale * skullOffset);
}

void MirrorApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32.Get());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.
	Pipelines::Basic->SetDirLights(mDirLights, 3);
	Pipelines::Basic->SetEyePosW(mEyePosW);
	Pipelines::Basic->SetFogColor(Colors::Black);
	Pipelines::Basic->SetFogStart(2.0f);
	Pipelines::Basic->SetFogRange(40.0f);

	switch (mRenderOptions)
	{
	case RenderOptions::Lighting:
		Pipelines::Basic->SetUseTexture(false);
		Pipelines::Basic->SetFogEnabled(false);
		break;
	case RenderOptions::Textures:
		Pipelines::Basic->SetUseTexture(true);
		Pipelines::Basic->SetFogEnabled(false);
		break;
	case RenderOptions::TexturesAndFog:
		Pipelines::Basic->SetUseTexture(true);
		Pipelines::Basic->SetFogEnabled(true);
		break;
	}

	// Draw the floor and walls to the back buffer as normal.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mRoomVB.GetAddressOf(), &stride, &offset);

	// Set per object constants.
	XMMATRIX world = XMLoadFloat4x4(&mRoomWorld);
	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world * view * proj;

	Pipelines::Basic->SetWorld(world);
	Pipelines::Basic->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Basic->SetWorldViewProj(worldViewProj);
	Pipelines::Basic->SetTexTransform(XMMatrixIdentity());
	Pipelines::Basic->SetMaterial(mRoomMat);

	// Floor
	Pipelines::Basic->SetDiffuseMap(mFloorDiffuseMapSRV);
	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->Draw(6, 0);

	// Wall
	Pipelines::Basic->SetDiffuseMap(mWallDiffuseMapSRV);
	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->Draw(18, 6);

	// Draw the skull to the back buffer as normal.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mSkullVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	world = XMLoadFloat4x4(&mSkullWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world * view * proj;

	Pipelines::Basic->SetWorld(world);
	Pipelines::Basic->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Basic->SetWorldViewProj(worldViewProj);
	Pipelines::Basic->SetMaterial(mSkullMat);

	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

	// Draw the mirror to stencil buffer only.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mRoomVB.GetAddressOf(), &stride, &offset);

	world = XMLoadFloat4x4(&mRoomWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world * view * proj;

	Pipelines::Basic->SetWorld(world);
	Pipelines::Basic->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Basic->SetWorldViewProj(worldViewProj);
	Pipelines::Basic->SetTexTransform(XMMatrixIdentity());

	// Do not write to render target.
	md3dImmediateContext->OMSetBlendState(RenderStates::NoRenderTargetWritesBS.Get(), blendFactor, 0xffffffff);

	// Render visible mirror pixels to stencil buffer.
	// Do not write mirror depth to depth buffer at this point,
	// otherwise it will occlude the reflection.
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::MarkMirrorDSS.Get(), 1);

	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->Draw(6, 24);

	// Restore states
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
	md3dImmediateContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);

	// Draw the skull reflection.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mSkullVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	world = XMLoadFloat4x4(&mSkullWorld) * R;
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world * view * proj;

	Pipelines::Basic->SetWorld(world);
	Pipelines::Basic->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Basic->SetWorldViewProj(worldViewProj);
	Pipelines::Basic->SetMaterial(mSkullMat);

	XMFLOAT3 oldLightDirections[3];
	for (int i = 0; i < 3; ++i) {
		oldLightDirections[i] = mDirLights[i].Direction;
		XMVECTOR lightDir = XMLoadFloat3(&mDirLights[i].Direction);
		XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&mDirLights[i].Direction, reflectedLightDir);
	}
	Pipelines::Basic->SetDirLights(mDirLights, 3);

	// Cull clockwise triangles for reflection.
	md3dImmediateContext->RSSetState(RenderStates::CullClockwiseRS.Get());

	// Only draw reflection into visible mirror pixels as marked by the stencil buffer.
	md3dImmediateContext->OMSetDepthStencilState(RenderStates::DrawReflectionDSS.Get(), 1);
	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

	// Restore default states.
	md3dImmediateContext->RSSetState(nullptr);
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);

	// Restore light directions.
	for (int i = 0; i < 3; ++i) {
		mDirLights[i].Direction = oldLightDirections[i];
	}

	Pipelines::Basic->SetDirLights(mDirLights, 3);

	// Draw the mirror to the back buffer as usual but with transparency
	// blending so the reflection shows through.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mRoomVB.GetAddressOf(), &stride, &offset);

	world = XMLoadFloat4x4(&mRoomWorld);
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world * view * proj;

	Pipelines::Basic->SetWorld(world);
	Pipelines::Basic->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Basic->SetWorldViewProj(worldViewProj);
	Pipelines::Basic->SetTexTransform(XMMatrixIdentity());
	Pipelines::Basic->SetMaterial(mMirrorMat);
	Pipelines::Basic->SetDiffuseMap(mMirrorDiffuseMapSRV);

	// Mirror
	md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS.Get(), blendFactor, 0xffffffff);
	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->Draw(6, 24);

	// Draw the skull shadow.
	md3dImmediateContext->IASetVertexBuffers(0, 1, mSkullVB.GetAddressOf(), &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mSkullIB.Get(), DXGI_FORMAT_R32_UINT, 0);

	XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMVECTOR toMainLight = -XMLoadFloat3(&mDirLights[0].Direction);
	XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
	XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

	// Set per object constants.
	world = XMLoadFloat4x4(&mSkullWorld) * S * shadowOffsetY;
	worldInvTranspose = MathHelper::InverseTranspose(world);
	worldViewProj = world * view * proj;

	Pipelines::Basic->SetWorld(world);
	Pipelines::Basic->SetWorldInvTranspose(worldInvTranspose);
	Pipelines::Basic->SetWorldViewProj(worldViewProj);
	Pipelines::Basic->SetMaterial(mShadowMat);

	// md3dImmediateContext->OMSetDepthStencilState(RenderStates::NoDoubleBlendDSS.Get(), 0);
	Pipelines::Basic->Apply(md3dImmediateContext.Get());
	md3dImmediateContext->DrawIndexed(mSkullIndexCount, 0, 0);

	// Restore default states.
	md3dImmediateContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
	md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);

	ThrowIfFailed(mSwapChain->Present(0, 0));
}

void MirrorApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void MirrorApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void MirrorApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void MirrorApp::OnMouseMove(WPARAM btnState, int x, int y)
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
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 50.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
