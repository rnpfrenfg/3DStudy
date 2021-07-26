#pragma once

#include "include.h"

class UploadBuffer
{
public:
	UploadBuffer() = default;
	~UploadBuffer()
	{
		if (mMappedData != nullptr)
		{
			mBuffer->Unmap(0, nullptr);
		}
		mMappedData = nullptr;
	}
	void Init(ID3D12Device* device, UINT elementCount, int elementSize, bool isConstantBuffer)
	{
		mElementSize = elementSize;

		if (isConstantBuffer)
		{
			mElementSize = CalcConstantBufferByteSize(mElementSize);
		}

		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = elementCount * mElementSize;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;


		DxThrowIfFailed(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mBuffer)));

		DxThrowIfFailed(mBuffer->Map(0, nullptr, (void**)&mMappedData));
	}

	void CopyToBuffer(int elementIndex, void* ptr)
	{
		memcpy(&mMappedData[elementIndex * mElementSize], ptr, mElementSize);
	}

	ComPtr<ID3D12Resource> mBuffer;

	UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}

private:

	BYTE* mMappedData = nullptr;

	UINT mElementSize;

};

