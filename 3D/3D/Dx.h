#pragma once

#include "include.h"

#include "Camera.h"
#include "CMeshObject.h"
#include "CommandBundle.h"
#include "GameSetting.h"
#include "GameTimer.h"
#include "Light.h"
#include "CTexture.h"
#include "CTextureManager.h"

#include <fstream>

#include "tempd3dx.h"

struct FrameResource
{
	DX::XMMATRIX view;
	DX::XMMATRIX InvView;
	DX::XMMATRIX Proj;
	DX::XMMATRIX InvProj;
	DX::XMMATRIX ViewProj;
	DX::XMMATRIX InvViewProj;
	DX::XMFLOAT3 EyePosW;
	float cbPerObjectPad1;
	DX::XMFLOAT2 RenderTargetSize;
	DX::XMFLOAT2 InvRenderTargetSize;
	float NearZ;
	float FarZ;
	float TotalTime;
	float DeltaTime;

	DX::XMFLOAT4 AmbientLight;

	DX::XMFLOAT4 fogColor = { 0.7f,0.7f,0.7f,1.0f };
	float gFogStart = 5.0f;
	float dFogRange = 150.0f;
	DX::XMFLOAT2 cbPerObjectPad2;

	Light Lights[MaxLights];
};

struct ObjectConstants
{
	DX::XMMATRIX world;
	DX::XMMATRIX TexTransform = DX::XMMatrixIdentity();
};

struct MaterialConstants
{
	DX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	DX::XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	DX::XMMATRIX MatTransform = DX::XMMatrixScaling(1,1,1);
};

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

	void InitDirectX();

	void CCreateDevice();
	void BuildPSO();
	void BuildRootSignature();
	void InitConstantBuffers();
	void InitCmdBundles();
	void LoadModels();
	void CCreateSwapChain();
	void CCreateCommandQueue(ComPtr<ID3D12CommandQueue>& que);
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

	UINT DrawRenderItems(const UINT startIndex, ComPtr<ID3D12GraphicsCommandList>& cmdList, std::vector<CMeshObject>& meshObjects);

private:

	CTextureManager texManager;

	Camera mMainCamera;

	static Dx* mApp;

	enum { SwapChainBufferCount = 2 };
	const float mMinDepth = 0.0f;
	const float mMaxDepth = 1.0f;
	
	ComPtr<IDXGIFactory4> mFactory;
	ComPtr<IDXGISwapChain> mSwapChain;
	ComPtr<ID3D12Device> mDevice;

	ComPtr<ID3D12RootSignature> mRootSignature;

	Mesh skullMesh;
	std::vector<CMeshObject> mMeshObjects;
	std::vector<CMeshObject> mTransMeshObjects;

	UploadBuffer<ObjectConstants> mObjectCB;
	UploadBuffer<MaterialConstants> mMaterialTestCB;
	UploadBuffer<FrameResource> mFrameCB;

	CTexture testTex;
	CTexture texWirefence;

	FrameResource frameResource;


	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandAllocator> mCommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	CommandBundle mDefaultGraphicsBundle;

	ComPtr<ID3D12Fence> mFence;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	ComPtr<ID3D12PipelineState> mPSO = nullptr;
	ComPtr<ID3D12PipelineState> mPsoBlend = nullptr;

	ComPtr<ID3D12Resource> mDepthStencilBuffer;
	ComPtr<ID3D12Resource> mRenderTargets[SwapChainBufferCount];

	UINT mRtvDescriptorSize;
	UINT mDsvDescriptorSize;
	UINT mCbvSrvUavDescriptorSize;

	bool m4xMsaaState = false;

	int mClientWidth = 800;
	int mClientHeight = 600;

	UINT mCurrBackBuffer = 0;

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