#include "RenderItem.h"

HRESULT RenderItem::_InitBundle(ComPtr<ID3D12Device> device, ComPtr<ID3D12PipelineState> pso)
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