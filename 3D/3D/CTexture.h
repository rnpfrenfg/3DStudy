#pragma once

#include "include.h"

#include "UploadBuffer.h"

class CTexture
{
public:
	SIZE_T index;
	ComPtr<ID3D12Resource> resource;
	UploadBufferBySize uploadBuffer;


	static HRESULT ReadFromDDSFile(const wchar_t* path, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12Device> device, _Out_ CTexture& texture)
	{
		return _ReadFromDDSFile(path, cmdList, device, texture);
	}

public:
	static HRESULT _ReadFromDDSFile(const wchar_t* path, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12Device> device, _Out_ CTexture& texture);
};

