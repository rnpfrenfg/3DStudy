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

	void _AddItem(RenderItem& item, ComPtr<ID3D12PipelineState> pso, std::vector<RenderItem>& list)
	{
		item._InitBundle(mDevice, pso);
		item._index = newObjIndex;
		list.push_back(item);
		newObjIndex++;
	}

	void AddRenderObject(RenderItem& item)
	{
		_AddItem(item, mPSO, mapData.mObjs);

		if (mContainsMirror)
		{
			_AddItem(item, mPsoDrawReflections, mapData.mReflectedObjs);
		}
		if (mShadow)
		{
			_AddItem(item, mPsoShadow, mapData.mShwdowObjs);
		}
	}
	void AddTransparent(RenderItem& item)
	{
		_AddItem(item, mPsoBlend, mapData.mTranses);
	}

	void AddConstRenderObject(RenderItem& item)
	{
		_AddItem(item, mPSO, mapData.mConstants);
		UpdateMatrix(item);
	}
	void AddMirror(RenderItem& item)
	{
		if (!mContainsMirror)
		{

		}

		_AddItem(item, mPsoMarkStencilMirrors, mapData.mMirrors);
	}

	void ChangeGraphicsSetting(GraphicSetting& setting)
	{
		graphicsSet = setting;

		frameResource.RenderTargetSize = DX::XMFLOAT2(graphicsSet.width, graphicsSet.height);
		frameResource.InvRenderTargetSize = DX::XMFLOAT2(1.0f / graphicsSet.width, 1.0f / graphicsSet.height);

		if (setting.shadow && !mShadow)
		{

		}
		mShadow = setting.shadow;

		//TODO
		mContainsMirror = true;
	}

	void Update(const GameTimer& gt)
	{
		UpdateFrameResource(gt);

		DX::XMVECTOR shadowPlane = DX::XMVectorSet(0, 1, 0, 0);

		DX::XMFLOAT3 light = frameResource.Lights[0].Direction;
		light.x = -light.x;
		light.y = -light.y;
		light.z = -light.z;
		DX::XMVECTOR toMainLight = DX::XMLoadFloat3(&light);

		DX::XMMATRIX S = DX::XMMatrixShadow(shadowPlane, toMainLight);
		DX::XMMATRIX shadowOffsetY = DX::XMMatrixTranslation(0, 0.001f, 0);
		S* shadowOffsetY;

		for (RenderItem& item : mapData.mObjs)
			UpdateMatrix(item);
		for (RenderItem& item : mapData.mConstants)
			UpdateMatrix(item);
		for (RenderItem& item : mapData.mTranses)
			UpdateMatrix(item);
		for (RenderItem& item : mapData.mMirrors)
			UpdateMatrix(item);
		for (RenderItem& item : mapData.mShwdowObjs)
			UpdateShadowMatrix(item);
		for (RenderItem& item : mapData.mReflectedObjs)
			UpdateReflectedMatrix(item);
	}

	void Draw(ComPtr<ID3D12GraphicsCommandList>& mCommandList)
	{
		mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

		mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress());
		mCommandList->SetGraphicsRootConstantBufferView(3, mMaterialTestCB.mBuffer->GetGPUVirtualAddress());

		mCommandList->SetPipelineState(mPSO.Get());

		DrawRenderItems(mCommandList, mapData.mObjs);
		DrawRenderItems(mCommandList, mapData.mConstants);

		if(mContainsMirror)
		{
			mCommandList->OMSetStencilRef(1);
			mCommandList->SetPipelineState(mPsoMarkStencilMirrors.Get());
			DrawRenderItems(mCommandList, mapData.mMirrors);

			mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress() + mFrameCB.ElementSize());
			mCommandList->SetPipelineState(mPsoDrawReflections.Get());
			DrawRenderItems(mCommandList, mapData.mReflectedObjs);

			mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress());
			mCommandList->OMSetStencilRef(0);
		}

		if (mShadow)
		{
			mCommandList->SetGraphicsRootConstantBufferView(3, mMaterialTestCB.mBuffer->GetGPUVirtualAddress() + mMaterialTestCB.ElementSize());
			mCommandList->SetPipelineState(mPsoBlend.Get());
			DrawRenderItems(mCommandList, mapData.mShwdowObjs);
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

