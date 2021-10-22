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
	DX::XMMATRIX _world = DX::XMMatrixIdentity();

	UINT _index;
	UINT _instanceCounts;
public:
	UINT dirty = FrameResource::FrameResources;

	Mesh* mesh = nullptr;
	CTexture* texture = nullptr;

	float x = 0;
	float y = 0;
	float z = 0;
	float scale = 1;
};