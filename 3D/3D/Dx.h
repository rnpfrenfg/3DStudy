#pragma once

#include "include.h"

#include "Camera.h"
#include "CMeshObject.h"
#include "CommandBundle.h"
#include "GameSetting.h"
#include "GameTimer.h"

#include <fstream>

class Dx
{
public:
	static Dx* GetApp();

public:
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	bool InitMainWindow();

	void CalculateFrameStats();

	HINSTANCE mhAppInst;
	HWND mhMainWnd;

	HANDLE mFenceEvent;
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

	void Set4xMsaaState(bool value);

	int Run();

private:

	void FlushCommandQueue();

	void LoadPipeline();
	void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);
	void CCreateSwapChain();
	void CCreateCommandQueue(ComPtr<ID3D12CommandQueue>& que);
	void CCreateRtvAndDsvDescriptorHeaps();

	void LoadAssets();
	void CCreateCommandBundles();

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

private:

	Camera mMainCamera;

	static Dx* mApp;

	enum { SwapChainBufferCount = 2 };
	const float mMinDepth = 0.0f;
	const float mMaxDepth = 1.0f;

	ComPtr<IDXGIFactory4> mFactory;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D12Device> mDevice;
		
	ComPtr<ID3D12RootSignature> mRootSignature;

	ID3D12DescriptorHeap* descriptorHeaps[1];
	ComPtr<ID3D12DescriptorHeap> mCbvHeap;
#define TempObjectCBType DX::XMFLOAT4X4
	UploadBuffer mObjectCB;

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	CommandBundle mDefaultGraphicsBundle;

	ComPtr<ID3D12Fence> mFence;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	CMeshObject meshTest;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	
	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12Resource> mRenderTargets[SwapChainBufferCount];

	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;

	bool m4xMsaaState = false;

	int mClientWidth = 800;
	int mClientHeight = 600;

	int mCurrBackBuffer = 0;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	UINT m4xMsaaQuality = 0;

	GameTimer mTimer;

	D3D12_VIEWPORT mScreenViewport;
	D3D12_RECT mScissorRect;

	UINT64 mCurrentFence = 0;

	bool mAppPaused = false;
	bool mResizing = false;

	float mRadius = 0.25;

	DX::XMMATRIX mWorld = DX::XMMatrixIdentity();
	DX::XMMATRIX mView = DX::XMMatrixIdentity();
	DX::XMMATRIX mProj = DX::XMMatrixIdentity();

private:
	GameSetting gameSetting;

	float cameraX = 0;
	float cameraY = 0;

	float mPhi = DX::XM_PIDIV4;
	float mTheta = 1.5f * DX::XM_PI;
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);
};
