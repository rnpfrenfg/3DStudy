#pragma once

#include "include.h"

template<typename T>
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
	}

	void Init(ID3D12Device* device, UINT elementCount, bool isConstant)
	{
		mElementSize = sizeof(T);

		if (isConstant == true)
		{
			mElementSize = CalcConstantBufferByteSize(mElementSize);
		}

		mWidth = mElementSize * elementCount;

		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = mWidth;
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

	UINT ElementSize()
	{
		return mElementSize;
	}

	UINT Width()
	{
		return mWidth;
	}

	void CopyToBuffer(void* ptr)
	{
		memcpy(mMappedData, ptr, mWidth);
	}

	void CopyToBuffer(int index, void* ptr)
	{
		memcpy(&mMappedData[index * mElementSize], ptr, sizeof(T));
	}

	ComPtr<ID3D12Resource> mBuffer;

private:

	UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}

	BYTE* mMappedData = nullptr;

	UINT mWidth;
	UINT mElementSize;
};

class UploadBufferBySize
{
public:
	UploadBufferBySize() = default;
	~UploadBufferBySize()
	{
		if (mMappedData != nullptr)
		{
			mBuffer->Unmap(0, nullptr);
		}
	}

	void Init(ID3D12Device* device, UINT elementSize, bool isConstant, UINT elementCount = 1)
	{
		mElementSize = elementSize;

		if (isConstant == true)
		{
			mElementSize = CalcConstantBufferByteSize(mElementSize);
		}

		mWidth = mElementSize * elementCount;

		D3D12_HEAP_PROPERTIES heapProps;
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = mWidth;
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

	UINT ElementSize()
	{
		return mElementSize;
	}

	UINT Width()
	{
		return mWidth;
	}

	void CopyToBuffer(void* ptr)
	{
		memcpy(mMappedData, ptr, mWidth);
	}

	void CopyToBuffer(int index, void* ptr)
	{
		memcpy(&mMappedData[index * mElementSize], ptr, mElementSize);
	}

	ComPtr<ID3D12Resource> mBuffer;

private:

	UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}

	BYTE* mMappedData = nullptr;

	UINT mWidth;
	UINT mElementSize;
};
