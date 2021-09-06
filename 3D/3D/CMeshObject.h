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
	void Init(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso, CTextureManager& texManager, CTexture* tex, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes)
	{
		mTex = tex;
		this->indexes = indexes;
		this->topology = topology;

		mVertexBuffer.Init(device.Get(), triangles, false);
		mVertexBuffer.CopyToBuffer(triangleVertices);
		mVertexBufferView.BufferLocation = mVertexBuffer.mBuffer->GetGPUVirtualAddress();
		mVertexBufferView.StrideInBytes = sizeof(Vertex);
		mVertexBufferView.SizeInBytes = mVertexBuffer.Width();

		mIndexBuffer.Init(device.Get(), indexes, false);
		mIndexBuffer.CopyToBuffer(indexList);
		mIndexBufferView.BufferLocation = mIndexBuffer.mBuffer->GetGPUVirtualAddress();
		mIndexBufferView.SizeInBytes = mIndexBuffer.Width();
		mIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

		T__initBundle(bundle, device, pso, texManager);
	}

	void T__initBundle(CommandBundle& bundle, ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso, CTextureManager& texManager)
	{
		bundle.Init(device, pso);

		{
			auto& cmdList = bundle.mCommandList;

			cmdList->IASetPrimitiveTopology(topology);
			cmdList->IASetIndexBuffer(&mIndexBufferView);
			cmdList->IASetVertexBuffers(0, 1, &mVertexBufferView);
			cmdList->DrawIndexedInstanced(indexes, 1, 0, 0, 0);

			cmdList->SetDescriptorHeaps(1, texManager.mSrvDescriptorHeap.GetAddressOf());
			//TODO texManager.DrawTexture(cmdList, tex);

			DxThrowIfFailed(cmdList->Close());
		}
	}

	CTexture* mTex;

	UploadBuffer<Vertex> mVertexBuffer;
	UploadBuffer<UINT32> mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	CommandBundle bundle;
	CommandBundle tempMirrorBundle;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	int indexes;
};

class CMeshObject
{
public:

	bool tempMirror;

	void BuildMAKKTRIS()//TODO
	{
		MAKKTRIS = DX::XMMatrixScaling(scale, scale, scale) * DX::XMMatrixTranslation(x, y, z);
		if (tempMirror)
		{
			DX::XMVECTOR mirrorPlane = DX::XMVectorSet(0, 0, 1, 0);
			DX::XMMATRIX R = DX::XMMatrixReflect(mirrorPlane);
			MAKKTRIS = MAKKTRIS * R;
		}
		MAKKTRIS = DX::XMMatrixTranspose(MAKKTRIS);
	}

	Mesh* mesh;

	float x;
	float y;
	float z;
	float scale = 1;

	DX::XMMATRIX MAKKTRIS;

private:
};