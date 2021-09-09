#pragma once

#include "include.h"

class CommandBundle
{
public:
	CommandBundle() = default;

	void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso)
	{
		DxThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&mCommandAllocator)));
		DxThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, mCommandAllocator.Get(), pso.Get(), IID_PPV_ARGS(&mCommandList)));
	}

	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
};

