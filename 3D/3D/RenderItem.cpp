#include "RenderItem.h"

void Mesh::Init(ComPtr<ID3D12Device> device, CTexture* tex, D3D12_PRIMITIVE_TOPOLOGY topology, Vertex* triangleVertices, int triangles, UINT32* indexList, int indexes)
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