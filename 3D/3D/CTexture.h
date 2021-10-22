#pragma once

#include "include.h"

#include "UploadBuffer.h"

class CTexture
{
public:
	SIZE_T index;
	ComPtr<ID3D12Resource> resource;
	UploadBufferBySize uploadBuffer;

	D3D12_GPU_DESCRIPTOR_HANDLE _handle = { 0, };
};
