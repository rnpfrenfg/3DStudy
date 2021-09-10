#pragma once

#include "include.h"

#include "CTexture.h"

class CTextureManager
{
	CTextureManager* singleton = nullptr;
public:
	CTextureManager() { if (singleton != nullptr) DxThrowIfFailed(-1); singleton = this; }

	HRESULT Init(ComPtr<ID3D12Device> device, UINT srvDescriptorSize);

	void AddTexture(CTexture& t);

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap;

private:
	ComPtr<ID3D12Device> mDevice;
	SIZE_T mSrvDescriptorSize;
	SIZE_T newTexIndex = 0;

	CTexture* lastTexture = nullptr;

	D3D12_SHADER_RESOURCE_VIEW_DESC tempDesc = {};
};

