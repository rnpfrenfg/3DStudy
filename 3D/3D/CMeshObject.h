#pragma once

#include "CommandBundle.h"
#include "Vertex.h"
#include "UploadBuffer.h"

#include "include.h"

struct Mesh
{
public:
	void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes)
	{
		mVertexBuffer.Init(device.Get(), triangles, false);
		mVertexBuffer.CopyToBuffer(triangleVertices);
		mVertexBufferView.BufferLocation = mVertexBuffer.mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.StrideInBytes = sizeof(Vertex);
		mVertexBufferView.SizeInBytes = mVertexBuffer.Size();

		mIndexBuffer.Init(device.Get(), indexes, false);
		mIndexBuffer.CopyToBuffer(indexList);
		mIndexBufferView.BufferLocation = mIndexBuffer.mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = mIndexBuffer.Size();
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

		bundle.Init(device, pso);

		{
			auto& cmdList = bundle.mCommandList;

			cmdList->IASetPrimitiveTopology(topology);
			cmdList->IASetIndexBuffer(&mIndexBufferView);
			cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
			cmdList->DrawIndexedInstanced(indexes, 1, 0, 0, 0);

			DxThrowIfFailed(cmdList->Close());
		}
	}

	UploadBuffer<Vertex> mVertexBuffer;
	UploadBuffer<UINT32> mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	CommandBundle bundle;
};

class CMeshObject
{
public:

	void BuildMAKKTRIS()
	{
		MAKKTRIS = DX::XMMatrixIdentity();
		// MAKKTRIS = DX::XMMatrixScaling(scale, scale, scale) * DX::XMMatrixTranslation(x, y, z);
		//MAKKTRIS = DX::XMMatrixTranspose(MAKKTRIS);
	}

	Mesh* mesh;

	float x;
	float y;
	float z;
	float scale = 1;

	DX::XMMATRIX MAKKTRIS;

private:
};