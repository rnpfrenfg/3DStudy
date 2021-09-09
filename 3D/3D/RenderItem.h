#pragma once

#include "include.h"

#include "CommandBundle.h"
#include "Vertex.h"
#include "UploadBuffer.h"
#include "CTexture.h"
#include "CTexture.h"
#include "CTextureManager.h"

struct Mesh
{
public:
	void Init(ComPtr<ID3D12Device> device, CTexture* tex, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes)
	{
		mTex = tex;
		this->indexes = indexes;
		this->topology = topology;

		mVertexBuffer.Init(device.Get(), triangles, false);
		mVertexBuffer.CopyAllToBuffer(triangleVertices);
		mVertexBufferView.BufferLocation = mVertexBuffer.mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.StrideInBytes = sizeof(Vertex);
		mVertexBufferView.SizeInBytes = mVertexBuffer.Width();

		mIndexBuffer.Init(device.Get(), indexes, false);
		mIndexBuffer.CopyAllToBuffer(indexList);
		mIndexBufferView.BufferLocation = mIndexBuffer.mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = mIndexBuffer.Width();
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	}

	CTexture* mTex;

	UploadBuffer<Vertex> mVertexBuffer;
	UploadBuffer<UINT32> mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	CommandBundle tempMirrorBundle;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	int indexes;
};

class RenderItem
{
public:

	bool dirty = true;

	Mesh* mesh = nullptr;

	float x = 0;
	float y = 0;
	float z = 0;
	float scale = 1;

	CommandBundle _bundle;
	UINT _index;
	UINT _indexShadow;//todo
	UINT _indexReflected;//only one mirror

	HRESULT _InitBundle(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso)
	{
		if (mesh == nullptr)
			return E_FAIL;

		_bundle.Init(device, pso);
		auto& cmdList = _bundle.mCommandList;

		cmdList->IASetPrimitiveTopology(mesh->topology);
		cmdList->IASetIndexBuffer(&mesh->mIndexBufferView);
		cmdList->IASetVertexBuffers(0, 1, &mesh->mVertexBufferView);
		cmdList->DrawIndexedInstanced(mesh->indexes, 1, 0, 0, 0);

		return cmdList->Close();
	}

private:
};