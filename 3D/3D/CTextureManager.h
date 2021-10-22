#pragma once

#include "include.h"

#include "CTexture.h"

class CTextureManager
{
	static CTextureManager* singleton;
public:
	CTextureManager() { if (singleton != nullptr) DxThrowIfFailed(-1); singleton = this; }

	static CTextureManager* GetInstance() { return singleton; }

	HRESULT Init(ComPtr<ID3D12Device> device, UINT srvDescriptorSize);

	int AddTexture(CTexture& t);
	int AddCubeMap(CTexture& t);

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

private:
	HRESULT BuildSrvDescriptorHeap();

	ComPtr<ID3D12Device> mDevice;
	int mSrvDescriptorSize;
	int maxTextures = 7;
	int newTexIndex = 0;

	CTexture* lastTexture = nullptr;

	D3D12_SHADER_RESOURCE_VIEW_DESC tempDesc = {};
};
