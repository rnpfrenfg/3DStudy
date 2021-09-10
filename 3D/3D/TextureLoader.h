#pragma once

#include "include.h"

#include "CTexture.h"

class TextureLoader
{
public:
	static HRESULT Load(const wchar_t* path, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12Device> device, _Out_ CTexture& texture);

private:
	static HRESULT _ReadFromDDSFile(const wchar_t* path, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12Device> device, _Out_ CTexture& texture);
};

