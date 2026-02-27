#include "TextureMgr.h"
#include <directxtk/DDSTextureLoader.h>

TextureMgr::TextureMgr() : md3dDevice(0)
{
}

TextureMgr::~TextureMgr()
{
	mTextureSRV.clear();
}

void TextureMgr::Init(ID3D11Device* device)
{
	md3dDevice = device;
}

ID3D11ShaderResourceView* TextureMgr::CreateTexture(std::wstring filename)
{
	ID3D11ShaderResourceView* srv = 0;

	// Does it already exist?
	if( mTextureSRV.find(filename) != mTextureSRV.end() )
	{
		srv = mTextureSRV[filename].Get();
	}
	else
	{
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(md3dDevice, filename.c_str(), nullptr, &srv));

		mTextureSRV[filename] = srv;
	}

	return srv;
}
 
