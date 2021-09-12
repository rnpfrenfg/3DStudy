#include "GPUQueue.h"

void GPUQueue::Init(ComPtr<ID3D12Device>& device)
{
	D3D12_COMMAND_QUEUE_DESC queDesc;
	queDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queDesc.NodeMask = 0;
	DxThrowIfFailed(device->CreateCommandQueue(&queDesc, IID_PPV_ARGS(commandQueue.GetAddressOf())));

	DxThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	mFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (mFenceEvent == NULL)
	{
		DxThrowIfFailed(-1);
	}
}

UINT64 GPUQueue::Signal()
{
	mCurrentFence++;
	commandQueue->Signal(mFence.Get(), mCurrentFence);
	return mCurrentFence;
}

void GPUQueue::WaitUntil(UINT64 fence)
{
	if (mFence->GetCompletedValue() < fence)
	{
		mFence->SetEventOnCompletion(fence, mFenceEvent);
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
}