#pragma once

#include "include.h"

#include "RenderItem.h"

class RenderManager
{
public:

	void Init(ComPtr<ID3D12Device>& device, ComPtr<ID3D12RootSignature>& rootSignature)
	{

	}

private:
	std::vector<RenderItem> mMeshObjects;
	std::vector<RenderItem> mTransMeshObjects;
	std::vector<RenderItem> mMirrors;
	std::vector<RenderItem> mReflectedObjs;
	std::vector<RenderItem> mShwdowObjs;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	ComPtr<ID3D12PipelineState> mPsoBlend = nullptr;
	//for marking stencil mirrors
	ComPtr<ID3D12PipelineState> mPsoMarkStencilMirrors = nullptr;
	ComPtr<ID3D12PipelineState> mPsoDrawReflections = nullptr;
	ComPtr<ID3D12PipelineState> mPsoShadow = nullptr;
};

