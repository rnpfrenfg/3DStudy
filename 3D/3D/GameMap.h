#pragma once

#include "include.h"

#include "GameTimer.h"
#include "GraphicSetting.h"
#include "RenderItem.h"
#include "GameMapData.h"
#include "GPUConstants.h"
#include "Camera.h"

//TODO : Rename
class GameMap
{
public:

	GameMap() = default;

	HRESULT Init(GraphicSetting& setting, ComPtr<ID3D12Device>& device);

	void AddRenderObject(RenderItem& item);

	void AddTransparent(RenderItem& item);

	void AddConstRenderObject(RenderItem& item);

	void AddMirror(RenderItem& item)
	{
		if (!mContainsMirror)
		{

		}

		_AddItem(item, mPsoMarkStencilMirrors, mapData.mirrors);
	}

	void ChangeGraphicsSetting(GraphicSetting& setting);

	void Update(const GameTimer& gt);

	void Draw(ComPtr<ID3D12GraphicsCommandList>& mCommandList)
	{
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress());
		mCommandList->SetGraphicsRootConstantBufferView(3, mMaterialTestCB.mBuffer->GetGPUVirtualAddress());

		mCommandList->SetPipelineState(mPSO.Get());

		DrawRenderItems(mCommandList, mapData.objs);
		DrawRenderItems(mCommandList, mapData.constObjs);

		if(mContainsMirror)
		{
			mCommandList->OMSetStencilRef(1);
			mCommandList->SetPipelineState(mPsoMarkStencilMirrors.Get());
			DrawRenderItems(mCommandList, mapData.mirrors);

			mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress() + mFrameCB.ElementSize());
			mCommandList->SetPipelineState(mPsoDrawReflections.Get());
			DrawRenderItems(mCommandList, mapData._reflected);

			mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress());
			mCommandList->OMSetStencilRef(0);
		}

		if (mShadow)
		{
			mCommandList->SetGraphicsRootConstantBufferView(3, mMaterialTestCB.mBuffer->GetGPUVirtualAddress() + mMaterialTestCB.ElementSize());
			mCommandList->SetPipelineState(mPsoBlend.Get());
			DrawRenderItems(mCommandList, mapData._shadows);
		}
	}

	Camera mMainCamera;

private:
	void UpdateFrameResource(const GameTimer& gt);
	void UpdateMatrix(RenderItem& item);
	void UpdateShadowMatrix(RenderItem& item);
	void UpdateReflectedMatrix(RenderItem& item);

	void DrawRenderItems(ComPtr<ID3D12GraphicsCommandList>& cmdList, std::vector<RenderItem>& meshObjects);

	void BuildRootSignature();
	void BuildPSO();

	void _AddItem(RenderItem& item, ComPtr<ID3D12PipelineState> pso, std::vector<RenderItem>& list);

	UploadBuffer<ObjectConstants> mObjectCB;
	UploadBuffer<MaterialConstants> mMaterialTestCB;
	UploadBuffer<FrameResource> mFrameCB;
	FrameResource frameResource;
	FrameResource reflectedFrameResoruce;

	GameMapData mapData;

	GraphicSetting graphicsSet;
	ComPtr<ID3D12Device> mDevice;

	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	ComPtr<ID3D12RootSignature> mRootSignature;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	ComPtr<ID3D12PipelineState> mPsoBlend = nullptr;
	ComPtr<ID3D12PipelineState> mPsoMarkStencilMirrors = nullptr;
	ComPtr<ID3D12PipelineState> mPsoDrawReflections = nullptr;
	ComPtr<ID3D12PipelineState> mPsoShadow = nullptr;

	bool mContainsMirror = false;
	bool mShadow = false;

	UINT newObjIndex;
};

