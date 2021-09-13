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
	void Init(ComPtr<ID3D12Device> device, CTexture* tex, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes);

	CTexture* mTex;

	UploadBuffer<Vertex> mVertexBuffer;
	UploadBuffer<UINT32> mIndexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;
	D3D12_INDEX_BUFFER_VIEW mIndexBufferView;

	D3D12_PRIMITIVE_TOPOLOGY topology;
	int indexes;
};

class RenderItem
{
public:
	UINT dirty = 3;

	Mesh* mesh = nullptr;

	float x = 0;
	float y = 0;
	float z = 0;
	float scale = 1;

	CommandBundle _bundle;
	UINT _index;
	DX::XMMATRIX _world = DX::XMMatrixIdentity();

	HRESULT _InitBundle(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso);

private:
};