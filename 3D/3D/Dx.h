#pragma once

#include "include.h"

#include "Camera.h"
#include "RenderItem.h"
#include "CommandBundle.h"
#include "GameSetting.h"
#include "GameTimer.h"
#include "Light.h"
#include "CTexture.h"
#include "CTextureManager.h"
#include "GraphicSetting.h"
#include "GPUQueue.h"
#include "GameMap.h"
#include "FrameResource.h"

#include "_work.h"//TODO

#include <fstream>

#include "tempd3dx.h"

class Dx
{
public:
	static Dx* GetApp();

public:
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool InitMainWindow();

	HINSTANCE mhAppInst;
	HWND mhMainWnd;

	std::wstring mClassName = L"MainWnd";
	std::wstring mMainWndCaption = L"d3d App";

	WORD mouseX;
	WORD mouseY;

public:
	Dx(HINSTANCE instance);
	~Dx();

	bool Init();

	void OnResize();

	void Update(const GameTimer& gt);

	void Render(const GameTimer& gt);

	ID3D12Resource* CurrentBackBuffer()const
	{
		return mRenderTargets[mCurrBackBuffer].Get();
	}

	int Run();

private:

	void InitDirectX();

	void CCreateDevice();
	void LoadModels();
	void CCreateSwapChain();
	void CCreateRtvAndDsvDescriptorHeaps();

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		handle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + mCurrBackBuffer * mRtvDescriptorSize;
		return handle;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
	{
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void FlushCommandQueue();
	void CalculateFrameStats();
	void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);
	void UpdateGraphicSetting(GraphicSetting& setting);

private:
	_work work;

	GPUQueue mQueue;
	GameMap mapRenderer;
	CTextureManager texManager;

	Camera mMainCamera;

	static Dx* mApp;

	enum { SwapChainBufferCount = 2 };
	
	ComPtr<IDXGIFactory4> mFactory;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D12Device> mDevice;

	Mesh skullMesh;

	CTexture testTex;
	CTexture texWirefence;
	CTexture texStone;

	UINT currFrameResourceIndex = 0;
	UINT mCmdAllocsFence[FrameResource::FrameResources];
	ComPtr<ID3D12CommandAllocator> mCommandAllocator[FrameResource::FrameResources];
	ComPtr<ID3D12GraphicsCommandList> mCommandList;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12Resource> mRenderTargets[SwapChainBufferCount];

	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;

	GraphicSetting graphicSetting;

	UINT mCurrBackBuffer = 0;

	GameTimer mTimer;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	bool mAppPaused = false;
	bool mResizing = false;

private:
	GameSetting gameSetting;

	float cameraX = 0;
	float cameraY = 0;

	float mPhi = DX::XM_PIDIV4;
	float mTheta = 1.5f * DX::XM_PI;
	float x = 0;
	float z = 0;
	float y = 0;
};