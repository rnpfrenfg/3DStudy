#pragma once

#include "CommandBundle.h"
#include "Vertex.h"
#include "UploadBuffer.h"

class CMeshObject
{
public:

	void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso, DXGI_FORMAT format)
	{
		this->device = device;

		//TODO
		Vertex triangleVertices[] = {
			{ { 0.0f, 0.25f, 0.0f}, { 1.0f, 0.0f, 0.0f, 1.0f } },
			{ { 0.5, -0.25f, 0}, { 0.0f, 1.0f, 0.0f, 1.0f } },
			{ { -0.5, -0.25f, 0}, { 0.0f, 0.0f, 1.0f, 1.0f } },
			{ {1, 0.25, 0}, {1,1,0,0}}
		};

		uint16_t indexList[] = {
			0,1,2, 3,1,0
		};

		mVertexBuffer.Init(device.Get(), 1, sizeof(triangleVertices), false);
		mVertexBuffer.CopyToBuffer(0, triangleVertices);
		//TODO CCreateBuffer(mVertexBuffer, sizeof(triangleVertices));
		//CCopyToBuffer(mVertexBuffer, triangleVertices, sizeof(triangleVertices));
		mVertexBufferView.BufferLocation = mVertexBuffer.mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.StrideInBytes = sizeof(Vertex);
		mVertexBufferView.SizeInBytes = sizeof(triangleVertices);

		mIndexBuffer.Init(device.Get(), 1, sizeof(indexList), false);
		mIndexBuffer.CopyToBuffer(0, indexList);
		mIndexBufferView.BufferLocation = mIndexBuffer.mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = sizeof(indexList);
		mIndexBufferView.Format = DXGI_FORMAT_R16_UINT;

		bundle.Init(device, pso);

		{
			auto& cmdList = bundle.mCommandList;

			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			cmdList->IASetIndexBuffer(&mIndexBufferView);
			cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
			cmdList->DrawIndexedInstanced(_countof(indexList), 1, 0, 0, 0);

			DxThrowIfFailed(cmdList->Close());
		}
	}

	CommandBundle bundle;

private:
	ComPtr<ID3D12Device> device;

	UploadBuffer mVertexBuffer;
	UploadBuffer mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;
};

