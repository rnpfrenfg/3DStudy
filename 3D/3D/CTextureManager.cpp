#include "CTextureManager.h"

HRESULT CTextureManager::Init(ComPtr<ID3D12Device> device, UINT srvDescriptorSize)
{
	mDevice = device;
	mSrvDescriptorSize = srvDescriptorSize;
	tempDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	tempDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	tempDesc.Texture2D.MostDetailedMip = 0;
	tempDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 3;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	return device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap));
}

void CTextureManager::AddTexture(CTexture& t)
{
	t.index = newTexIndex;

	tempDesc.Format = t.resource->GetDesc().Format;
	tempDesc.Texture2D.MipLevels = t.resource->GetDesc().MipLevels;

	D3D12_CPU_DESCRIPTOR_HANDLE hDescriptor = mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	hDescriptor.ptr += mSrvDescriptorSize * newTexIndex;

	mDevice->CreateShaderResourceView(t.resource.Get(), &tempDesc, hDescriptor);

	newTexIndex++;

	t._handle = mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	t._handle.ptr += t.index * mSrvDescriptorSize;
}