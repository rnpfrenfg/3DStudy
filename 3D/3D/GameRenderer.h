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
#include "CommandBundle.h"
#include "GameObject.h"

//TODO : Rename
class GameRenderer
{
public:
	GameRenderer() = default;

	HRESULT Init(GraphicSetting& setting, ComPtr<ID3D12Device>& device);

	void AddRenderItem(RenderItem* item);
	void AddGameObject(GameObject& item);

	void AddConstRenderObject(RenderItem& item);

	void ChangeGraphicsSetting(GraphicSetting& setting);

	void Update(const GameTimer& gt, GPUQueue& queue);

	void Draw(ComPtr<ID3D12GraphicsCommandList> cmdList, GPUQueue& queue);

	Camera mMainCamera;

private:
	void BuildRootSignature();
	void BuildPSO();
	void DefineSkullAnimation();

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateFrameResource(const GameTimer& gt);
	void UpdateGameObjectsMatrix(std::vector<GameObject>& list);

	void DrawRenderItems(ComPtr<ID3D12GraphicsCommandList>& cmdList, std::vector<RenderItem*>& renderItems);

	GameMapData mapData;
	FrameResource mFrameResources[FrameResource::FrameResources];
	FrameResource* mCurrFrameResource;
	int currFrameResourceIndex = 0;

	GraphicSetting graphicsSet;
	ComPtr<ID3D12Device> mDevice;

	ComPtr<ID3D12RootSignature> mRootSignature;
	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	ComPtr<ID3D12PipelineState> mPsoCompute;

	static D3D12_GPU_DESCRIPTOR_HANDLE lastTexHandle;

	std::vector<RenderItem*> renderItems;

	bool mContainsMirror = false;
	bool mShadow = false;

	float mAnimTimePos = 0;
	BoneAnimation mSkullAnimation;
};

