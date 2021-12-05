#pragma once

#include "include.h"

#include "Vertex.h"
#include "UploadBuffer.h"
#include "CTexture.h"
#include "CTextureManager.h"

#include "FrameResource.h"

struct Mesh
{
public:
	void Init(ComPtr<ID3D12Device> device, CTexture* tex, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes);

	CTexture* mTex;

	UploadBuffer<Vertex> mVertexBuffer;
	UploadBuffer<UINT32> mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	int indexes;

	DX::BoundingBox _bounds;//TODO,. 
};

class RenderItem
{
public:
	RenderItem() = default;
	RenderItem(const RenderItem & rhs) = delete;

	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Mesh* mesh = nullptr;
	//CTexture* texture = nullptr;

public:
	DX::XMMATRIX world = DX::XMMatrixIdentity();

	DX::BoundingBox Bounds;

	int gpuIndex = 0;
	int instances = 0;
};