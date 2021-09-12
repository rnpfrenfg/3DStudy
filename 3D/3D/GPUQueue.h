#pragma once

#include "include.h"

class GPUQueue
{
public:
	~GPUQueue()
	{
		CloseHandle(mFenceEvent);
	}

	void Init(ComPtr<ID3D12Device>& device);

	UINT64 Signal();

	void WaitUntil(UINT64 fence);

	ComPtr<ID3D12CommandQueue> commandQueue;

private:
	HANDLE mFenceEvent = NULL;

	ComPtr<ID3D12Fence> mFence;
	UINT64 mCurrentFence = 0;
};

