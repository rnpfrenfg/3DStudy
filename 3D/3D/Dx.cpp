#include "Dx.h"

#include <d3dcompiler.h>
#include "Vertex.h"

Dx* Dx::mApp = nullptr;

LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return Dx::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

void Transition(D3D12_RESOURCE_BARRIER& barrier,
	_In_ ID3D12Resource* pResource,
	D3D12_RESOURCE_STATES stateBefore,
	D3D12_RESOURCE_STATES stateAfter,
	UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
	D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
{
	ZeroMemory(&barrier, sizeof(barrier));
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = flags;
	barrier.Transition.pResource = pResource;
	barrier.Transition.StateBefore = stateBefore;
	barrier.Transition.StateAfter = stateAfter;
	barrier.Transition.Subresource = subresource;
}

Dx::Dx(HINSTANCE instance)
{
	assert(mApp == nullptr);
	mApp = this;
	mhAppInst = instance;

	gameSetting.mouseSensivity = 0.5;//TODO
}

Dx::~Dx()
{
	CloseHandle(mFenceEvent);
	UnregisterClass(mClassName.c_str(), mhAppInst);
}

LRESULT Dx::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		return 0;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));  TODO
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam........);
		return 0;
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, ........);
	{
		WORD newX = LOWORD(lParam);
		float difX = newX - mouseX;
		difX = DX::XMConvertToRadians(difX * gameSetting.mouseSensivity);

		WORD newY = HIWORD(lParam);
		float difY = newY - mouseY;
		difY = DX::XMConvertToRadians(difY * gameSetting.mouseSensivity);

		mMainCamera.Rotate(difX, difY);

		mouseX = newX;
		mouseY = newY;

	}
	return 0;
	case WM_CHAR:
		std::cout << (char)wParam;
		switch ((char)wParam)
		{
		case 'w':
		case 'W':
			z -= 1;
			return 0;
		case 's':
		case 'S':
			z += 1;
			return 0;
		case 'a':
		case 'A':
			x += 1;
			return 0;
		case 'd':
		case 'D':
			x -= 1;
			return 0;
		}
	case WM_KEYDOWN:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

Dx* Dx::GetApp()
{
	return mApp;
}

bool Dx::Init()
{
#if CDEBUG
	{
		ComPtr<ID3D12Debug> debugController;
		DxThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();

		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	}
#endif

	if (!InitMainWindow())
		return false;

	mFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (mFenceEvent == NULL)
	{
		DxThrowIfFailed(-1);
	}

	LoadPipeline();
	LoadAssets();

	OnResize();

	{
		CURSORINFO cursor;
		GetCursorInfo(&cursor);
		mouseX = cursor.ptScreenPos.x;
		mouseY = cursor.ptScreenPos.y;
	}

	mMainCamera.SetProj(mClientWidth, mClientHeight);
	//mMainCamera.SetRotate(1, 0);
	z = 5;

	return true;
}

bool Dx::InitMainWindow()
{
	WNDCLASSEX winc;
	winc.cbSize = sizeof(winc);
	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.hInstance = mhAppInst;
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winc.cbClsExtra = 0;
	winc.cbWndExtra = 0;
	winc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = mClassName.c_str();
	winc.lpfnWndProc = winProc;

	if (!(RegisterClassEx(&winc)))
	{
		return false;
	}

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;


	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	SetWindowPos(mhMainWnd, HWND_TOP, 1000, 400, mClientWidth, mClientHeight, SWP_SHOWWINDOW);

	return true;
}

void Dx::LoadPipeline()
{
	DxThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&mFactory)));

	ComPtr<IDXGIAdapter1> hardwareAdapter;
	GetHardwareAdapter(mFactory.Get(), &hardwareAdapter);
	HRESULT result = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice));
	if (FAILED(result))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		DxThrowIfFailed(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		DxThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mDevice)
		));
	}

	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CCreateCommandQueue(mCommandQueue);
	CCreateSwapChain();
	CCreateRtvAndDsvDescriptorHeaps();
}

void Dx::CCreateSwapChain()
{
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	DxThrowIfFailed(mFactory->CreateSwapChain(mCommandQueue.Get(), &sd, mSwapChain.GetAddressOf()));
}

void Dx::CCreateCommandQueue(ComPtr<ID3D12CommandQueue>& que)
{
	D3D12_COMMAND_QUEUE_DESC queDesc;
	queDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//todo
	queDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	queDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queDesc.NodeMask = 0;
	DxThrowIfFailed(mDevice->CreateCommandQueue(&queDesc, IID_PPV_ARGS(que.GetAddressOf())));
}

void Dx::CCreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	DxThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		DxThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i])));
		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHeapHandle);

		rtvHeapHandle.ptr += 1 * mRtvDescriptorSize;
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	DxThrowIfFailed(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void Dx::LoadAssets()
{
	{
		enum{slots = 3};
		D3D12_ROOT_PARAMETER slotRootParameter[slots];

		slotRootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		slotRootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		slotRootParameter[0].Descriptor.ShaderRegister = 0;
		slotRootParameter[0].Descriptor.RegisterSpace = 0;

		slotRootParameter[1] = slotRootParameter[0];
		slotRootParameter[1].Descriptor.ShaderRegister = 1;

		slotRootParameter[2] = slotRootParameter[0];
		slotRootParameter[2].Descriptor.ShaderRegister = 2;

		D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
		rootSigDesc.NumParameters = slots;
		rootSigDesc.pParameters = slotRootParameter;
		rootSigDesc.NumStaticSamplers = 0;
		rootSigDesc.pStaticSamplers = nullptr;
		rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ComPtr<ID3DBlob> serializedRootsig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;

		DxThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootsig.GetAddressOf(), errorBlob.GetAddressOf()));
		DxThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootsig->GetBufferPointer(), serializedRootsig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));

	}

	{
#if CDEBUG
		UINT compileFlags = 0;// D3DCOMPILE_DEBUG;
#else
		UINT compileFlags = 0;
#endif

		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

		DxThrowIfFailed(D3DCompileFromFile(L"shaders.txt", nullptr, nullptr, "VS", "vs_5_1", compileFlags, 0, &vertexShader, nullptr));
		DxThrowIfFailed(D3DCompileFromFile(L"shaders.txt", nullptr, nullptr, "PS", "ps_5_1", compileFlags, 0, &pixelShader, nullptr));

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0, };
		psoDesc.InputLayout.NumElements = _countof(inputElementDescs);
		psoDesc.InputLayout.pInputElementDescs = inputElementDescs;


		psoDesc.pRootSignature = mRootSignature.Get();

#define C_ADD_SHADER(a,b) psoDesc.a.pShaderBytecode = b->GetBufferPointer(); psoDesc.a.BytecodeLength = b->GetBufferSize();
		C_ADD_SHADER(VS, vertexShader);
		C_ADD_SHADER(PS, pixelShader);
#undef C_ADD_SHADER

		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
		psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		psoDesc.RasterizerState.DepthClipEnable = TRUE;
		psoDesc.RasterizerState.MultisampleEnable = FALSE;// TODO
		psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;//
		psoDesc.RasterizerState.ForcedSampleCount = 0;//
		psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
		psoDesc.BlendState.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			D3D12_RENDER_TARGET_BLEND_DESC& blendDesc = psoDesc.BlendState.RenderTarget[i];
			blendDesc.BlendEnable = FALSE;
			blendDesc.LogicOpEnable = FALSE;

			blendDesc.SrcBlend = D3D12_BLEND_ONE;
			blendDesc.DestBlend = D3D12_BLEND_ZERO;
			blendDesc.BlendOp = D3D12_BLEND_OP_ADD;

			blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;

			blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
			blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}

		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.StencilEnable = TRUE;
		psoDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		psoDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

		psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		psoDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		psoDesc.DepthStencilState.BackFace = psoDesc.DepthStencilState.FrontFace;

		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = mBackBufferFormat;
		psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = mDepthStencilFormat;

		DxThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
	}

	{

		mFrameCB.Init(mDevice.Get(), 1, true);

		mObjectCB.Init(mDevice.Get(), 20000, true);//TODO

		mMaterialTestCB.Init(mDevice.Get(), 1, true);

		MaterialConstants tetst;
		tetst.DiffuseAlbedo = DX::XMFLOAT4(0.2f, 0.6f, 0.6f, 1.0f);
		tetst.FresnelR0 = DX::XMFLOAT3(0.01f, 0.01f, 0.01f);
		tetst.Roughness = 0.125f;

		mMaterialTestCB.CopyToBuffer(&tetst);
	}

	DxThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	DxThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator)));
	DxThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator.Get(), mPSO.Get(), IID_PPV_ARGS(&mCommandList)));
	mCommandList->Close();

	CCreateCommandBundles();

	{
		std::ios_base::sync_with_stdio(false);
		std::cout.tie(NULL);
		using std::cout;

		cout << "READ" << '\n';

		std::ifstream fin;
		fin.open("Models/skull.txt", std::ios::in);
		if (fin.fail())
		{
			DxThrowIfFailed(-1);
		}

		int vertexes;
		int indexes;

		fin >> vertexes >> indexes;
		cout << vertexes << '\n' << indexes;;

		Vertex* vertexList = new Vertex[vertexes];
		for (int i = 0; i < vertexes; i++)
		{
			fin >> vertexList[i].position.x;
			fin >> vertexList[i].position.y;
			fin >> vertexList[i].position.z;

			fin >> vertexList[i].normal.x;
			fin >> vertexList[i].normal.y;
			fin >> vertexList[i].normal.z;
		}
		UINT32* indexList = new UINT32[indexes * 3];
		for (int i = 0; i < indexes * 3; i++)
		{
			fin >> indexList[i];
		}

		skullMesh.Init(mDevice, mPSO, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vertexList, vertexes, indexList, 3 * indexes);

		delete[] indexList;
		delete[] vertexList;
		fin.close();

		for (int i = 0; i < 3; i++)
		{
			CMeshObject obj;
			obj.x = obj.z = 10 * i;
			obj.y = 0;
			obj.scale = 0.5;
			obj.mesh = &skullMesh;
			mMeshObjects.push_back(obj);
		}

		cout << "END\n";
	}
}

void Dx::CCreateCommandBundles()
{
	mDefaultGraphicsBundle.Init(mDevice, mPSO);

	{
		auto& cmdList = mDefaultGraphicsBundle.mCommandList;

		{
			cmdList->SetGraphicsRootSignature(mRootSignature.Get());

			cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		}

		DxThrowIfFailed(cmdList->Close());
	}
}

void Dx::FlushCommandQueue()
{
	mCurrentFence++;

	DxThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		DxThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, mFenceEvent));
		WaitForSingleObject(mFenceEvent, INFINITE);
	}
}

void Dx::GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
{
	*ppAdapter = nullptr;
	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		IDXGIAdapter1* pAdapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter))
		{
			break;
		}

		if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			*ppAdapter = pAdapter;
			return;
		}
		pAdapter->Release();
	}
}

int Dx::Run()
{
	MSG msg{ 0, };
	mTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else//TODO
		{
			mTimer.Tick();
			if (!mAppPaused)
			{
				CalculateFrameStats();
				Update(mTimer);
				Render(mTimer);
			}
			else
			{
				Sleep(10);
			}
		}
	}
	return (int)msg.wParam;
}

void Dx::CalculateFrameStats()
{
	static int frameCnt = 0;;
	static float timeElapsed = 0.0f;
	frameCnt++;

	if (mTimer.TotalTime() - timeElapsed >= 1.0f)
	{
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = mMainWndCaption + L"  fps: " + fpsStr + L"   mspf: " + mspfStr;

		SetWindowText(mhMainWnd, windowText.c_str());

		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void Dx::Render(const GameTimer& gt)
{

	FlushCommandQueue();

	DxThrowIfFailed(mCommandAllocator->Reset());
	DxThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), mPSO.Get()));

	D3D12_RESOURCE_BARRIER barrier;

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	Transition(barrier, CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	rtvHandle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + mCurrBackBuffer * mRtvDescriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE dsView = DepthStencilView();
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsView);

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DX::Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	mCommandList->ExecuteBundle(mDefaultGraphicsBundle.mCommandList.Get());

	mCommandList->SetGraphicsRootConstantBufferView(2, mFrameCB.mBuffer->GetGPUVirtualAddress());
	mCommandList->SetGraphicsRootConstantBufferView(1, mMaterialTestCB.mBuffer->GetGPUVirtualAddress());

	for (UINT i = 0; i < mMeshObjects.size(); i++)
	{
		auto& mesh = mMeshObjects[i];

		mesh.BuildMAKKTRIS();
		mObjectCB.CopyToBuffer(i, &mesh.MAKKTRIS);

		mCommandList->SetGraphicsRootConstantBufferView(0, mObjectCB.mBuffer->GetGPUVirtualAddress() + mObjectCB.ElementSize() * i);

		mCommandList->ExecuteBundle(mesh.mesh->bundle.mCommandList.Get());
	}

	Transition(barrier, CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &barrier);

	DxThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	DxThrowIfFailed(mSwapChain->Present(0, 0));

	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
}

void Dx::OnResize()
{
	FlushCommandQueue();

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, mClientWidth, mClientHeight };

	DxThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		mRenderTargets[i].Reset();
	mDepthStencilBuffer.Reset();

	// Resize the swap chain.
	DxThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mClientWidth, mClientHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	mCurrBackBuffer = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		DxThrowIfFailed(mSwapChain->GetBuffer(
			i, IID_PPV_ARGS(&mRenderTargets[i])
		));
		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.ptr += 1 * mRtvDescriptorSize;
	}

	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	D3D12_HEAP_PROPERTIES properties;//TODO
	properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	properties.CreationNodeMask = 1;
	properties.VisibleNodeMask = 1;
	DxThrowIfFailed(mDevice->CreateCommittedResource(
		&properties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	D3D12_RESOURCE_BARRIER barrier;
	Transition(barrier, mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mCommandList->ResourceBarrier(1, &barrier);

	DxThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void Dx::Update(const GameTimer& gt)
{
	using DX::XMVECTOR;
	using DX::XMMATRIX;

	mMainCamera.SetPos(x, y, z);
	mMainCamera.SetView();

	XMMATRIX worldViewProj = mWorld * mMainCamera.mView * mMainCamera.mProj;

	worldViewProj = DX::XMMatrixTranspose(worldViewProj);

	{

		XMMATRIX view = mMainCamera.mView;
		XMMATRIX proj = mProj;

		XMMATRIX viewProj = view * proj;
		XMMATRIX invView = DX::XMMatrixInverse(nullptr, view);//TODO maybe error because different with book
		XMMATRIX invProj = DX::XMMatrixInverse(nullptr, proj);
		XMMATRIX invViewProj = DX::XMMatrixInverse(nullptr, viewProj);

		frameResource.view = DX::XMMatrixTranspose(view);
		frameResource.InvView = DX::XMMatrixTranspose(invView);
		frameResource.Proj = DX::XMMatrixTranspose(proj);
		frameResource.InvProj = DX::XMMatrixTranspose(invProj);
		frameResource.ViewProj = DX::XMMatrixTranspose(viewProj);
		frameResource.InvViewProj = DX::XMMatrixTranspose(invViewProj);

		DX::XMStoreFloat3(&frameResource.EyePosW, mMainCamera.target);
		frameResource.RenderTargetSize = DX::XMFLOAT2(mClientWidth, mClientHeight);
		frameResource.InvRenderTargetSize = DX::XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
		frameResource.NearZ = mMainCamera.mNearZ;
		frameResource.FarZ = mMainCamera.mFarZ;
		frameResource.TotalTime = mTimer.TotalTime();
		frameResource.DeltaTime = mTimer.DeltaTime();
		
		frameResource.AmbientLight = { 0.25f,0.25f,0.35f,1.0f };
		frameResource.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		frameResource.Lights[0].Strength = { 0.6f,0.6f,0.6f };
		frameResource.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		frameResource.Lights[1].Strength = { 0.3f,0.3f,0.3f };
		frameResource.Lights[2].Direction = { 0.0f,-0.707f,-0.707f };
		frameResource.Lights[2].Strength = { 0.15f,0.15f,0.15f };

		mFrameCB.CopyToBuffer(&frameResource);
	}
}

void Dx::Set4xMsaaState(bool value)
{
	if (m4xMsaaState != value)
	{
		m4xMsaaState = value;

		CCreateSwapChain();
		OnResize();
	}
}