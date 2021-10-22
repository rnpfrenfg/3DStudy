#pragma once

#include "include.h"

#pragma comment(lib, "libfbxsdk.lib")
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "libxml2-md.lib")
#pragma comment(lib, "libxml2-mt.lib")
#pragma comment(lib, "zlib-md.lib")
#pragma comment(lib, "zlib-mt.lib")

#include <fbxsdk.h>

#include "RenderItem.h"
#include "AnimationHelper.h"
#include "Vertex.h"

HRESULT _ReadFromFBXFile(const wchar_t* path, ComPtr<ID3D12GraphicsCommandList> cmdList, ComPtr<ID3D12Device> device, SkinnedData& skinned);

class FbxLoader
{
public:
	HRESULT Init();

	HRESULT LoadFBX(const wchar_t* path, SkinnedData& skinned);

private:
	FbxManager* manager;
	FbxIOSettings* ioSet;
	bool fbxNonblocking = false;
};

