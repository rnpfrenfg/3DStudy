#pragma once

#include "CommandBundle.h"
#include "Vertex.h"
#include "UploadBuffer.h"

struct Material
{
	std::string name;
	int cbIndex;
	int diffuseSrvHeapIndex;//chapter 9, todo
	int numFramesDirty;
	DX::XMFLOAT4 diffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	DX::XMFLOAT3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float roughness = 0.25f;
	//DX::XMFLOAT4X4 matTransform = DX::XMMatrixIdentity();
};

class CMeshObject
{
public:

	void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes)
	{
		this->device = device;

		using DX::XMFLOAT3;
		using DX::XMFLOAT4;
		namespace Colors = DX::Colors;

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

	CommandBundle bundle;

private:
	ComPtr<ID3D12Device> device;

	UploadBuffer<Vertex> mVertexBuffer;
	UploadBuffer<UINT32> mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	D3D12_PRIMITIVE_TOPOLOGY mTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

