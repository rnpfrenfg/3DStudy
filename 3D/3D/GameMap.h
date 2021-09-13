#pragma once

#include "include.h"

#include "GameTimer.h"
#include "GraphicSetting.h"
#include "RenderItem.h"
#include "GameMapData.h"
#include "FrameResource.h"
#include "Camera.h"
#include "GPUQueue.h"
#include "AnimationHelper.h"

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

	void Update(const GameTimer& gt, GPUQueue& queue);

	void Draw(ComPtr<ID3D12GraphicsCommandList> cmdList, GPUQueue& queue);

	Camera mMainCamera;

private:
	void BuildRootSignature();
	void BuildPSO();
	void DefineSkullAnimation();

	void UpdateFrameResource(const GameTimer& gt);
	void UpdateMatrix(RenderItem& item, UploadBuffer<ObjectConstants>& objectCB);
	void UpdateShadowMatrix(RenderItem& item, UploadBuffer<ObjectConstants>& objectCB);
	void UpdateReflectedMatrix(RenderItem& item, UploadBuffer<ObjectConstants>& objectCB);
	void UpdateScene(float dt);

	void DrawRenderItems(ComPtr<ID3D12GraphicsCommandList>& cmdList, std::vector<RenderItem>& meshObjects);

	void _AddItem(RenderItem& item, ComPtr<ID3D12PipelineState> pso, std::vector<RenderItem>& list);

	GameMapData mapData;
	FrameResource mFrameResources[FrameResource::FrameResources];
	FrameResource* mCurrFrameResource;
	int currFrameResourceIndex = 0;

	GraphicSetting graphicsSet;
	ComPtr<ID3D12Device> mDevice;

	ComPtr<ID3D12RootSignature> mRootSignature;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	ComPtr<ID3D12PipelineState> mPsoBlend = nullptr;
	ComPtr<ID3D12PipelineState> mPsoMarkStencilMirrors = nullptr;
	ComPtr<ID3D12PipelineState> mPsoDrawReflections = nullptr;
	ComPtr<ID3D12PipelineState> mPsoShadow = nullptr;

	ComPtr<ID3D12PipelineState> mPsoCompute;

	bool mContainsMirror = false;
	bool mShadow = false;

	UINT newObjIndex;

	float mAnimTimePos = 0;
	BoneAnimation mSkullAnimation;
};

