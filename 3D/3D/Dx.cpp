#include "Dx.h"

#include "Vertex.h"
#include "TextureLoader.h"

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

	gameSetting.mouseSensivity = 0.5;
}

Dx::~Dx()
{
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
	{
		WORD newX = LOWORD(lParam);
		WORD newY = HIWORD(lParam);
		mouseDown.x = newX;
		mouseDown.y = newY;
		return 0;
	}
	case WM_MBUTTONDOWN:
		return 0;
	case WM_RBUTTONDOWN:
	{
		std::cout << mMainCamera.mPos.m128_f32[0]<<' '<< mMainCamera.mPos.m128_f32[1]<<' '<< mMainCamera.mPos.m128_f32[2]<<'\n';
	}
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	case WM_MOUSEMOVE:
	{
		WORD newX = LOWORD(lParam);
		WORD newY = HIWORD(lParam);
		if ((wParam & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = DX::XMConvertToRadians(0.25f * static_cast<float>(newX - mouseDown.x));
			float dy = DX::XMConvertToRadians(0.25f * static_cast<float>(newY - mouseDown.y));

			mapRenderer.mMainCamera.Pitch(dy);
			mapRenderer.mMainCamera.RotateY(dx);
		}

		mouseDown.x = newX;
		mouseDown.y = newY;
	}
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

	InitDirectX();

	OnResize();
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

	RECT R = { 0, 0, graphicSetting.width, graphicSetting.height };
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

	SetWindowPos(mhMainWnd, HWND_TOP, 1000, 400, graphicSetting.width, graphicSetting.height, SWP_SHOWWINDOW);

	return true;
}

void Dx::InitDirectX()
{
#if CDEBUG
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		DxThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
	DxThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&mFactory)));

	CCreateDevice();

	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mQueue.Init(mDevice);
	CCreateSwapChain();
	CCreateRtvAndDsvDescriptorHeaps();

	for (int i = 0; i < FrameResource::FrameResources; i++)
	{
		DxThrowIfFailed(mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocator[i])));
	}
	DxThrowIfFailed(mDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocator[0].Get(), nullptr, IID_PPV_ARGS(&mCommandList)));

	graphicSetting.shadow = false;
	graphicSetting.mirror = false;
	mapRenderer.Init(graphicSetting, mDevice, &m_game);

	DxThrowIfFailed(texManager.Init(mDevice, mCbvSrvUavDescriptorSize));

	{//LoadTextures
		DxThrowIfFailed(TextureLoader::Load(L"sample.dds", mCommandList, mDevice, testTex));
		texManager.AddTexture(testTex);
		DxThrowIfFailed(TextureLoader::Load(L"wirefence.dds", mCommandList, mDevice, texWirefence));
		texManager.AddTexture(texWirefence);
		DxThrowIfFailed(TextureLoader::Load(L"stone.dds", mCommandList, mDevice, texStone));
		texManager.AddTexture(texStone);
		DxThrowIfFailed(TextureLoader::Load(L"cubemap.dds", mCommandList, mDevice, texCubemap));
		texManager.AddCubeMap(texCubemap);
	}

	DxThrowIfFailed(mCommandList->Close());
	mQueue.commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)mCommandList.GetAddressOf());
	FlushCommandQueue();

	LoadModels();
}

void Dx::CCreateDevice()
{
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
}

void Dx::CCreateSwapChain()
{
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = graphicSetting.width;
	sd.BufferDesc.Height = graphicSetting.height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = graphicSetting.backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = graphicSetting.m4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = graphicSetting.m4xMsaaState ? (graphicSetting.m4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	DxThrowIfFailed(mFactory->CreateSwapChain(mQueue.commandQueue.Get(), &sd, mSwapChain.GetAddressOf()));
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


void Dx::LoadModels()
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

		vertexList[i].TexC.x = rand() / (float)RAND_MAX;
		vertexList[i].TexC.y = rand() / (float)RAND_MAX;
	}
	UINT32* indexList = new UINT32[indexes * 3];
	for (int i = 0; i < indexes * 3; i++)
	{
		fin >> indexList[i];
	}

	skullMesh.Init(mDevice, &texWirefence, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vertexList, vertexes, indexList, 3 * indexes);

	delete[] indexList;
	delete[] vertexList;
	fin.close();

	RenderItem* skullR = new RenderItem;
	skullR->mesh = &skullMesh;
	mapRenderer.AddRenderItem(skullR);

	skullObject.item = skullR;
	for (int i = 0; i < 5; i++)
	{
		auto& position = skullObject.transform.position;

		skullObject.transform.scale = 0.5;
		position.x = position.z = 10 * i;
		position.y = 0;

		mapRenderer.AddGameObject(skullObject);
	}

	Vertex tempV[4];

	tempV[0].position.x = tempV[0].position.y = tempV[0].position.z = 0;
	tempV[0].normal = { 0.572276, 0.816877, 0.0721907 };
	tempV[0].TexC = { 0,1 };
	tempV[1] = tempV[2] = tempV[3] = tempV[0];

	tempV[1].position.y = 1;
	tempV[1].TexC = { 0,0 };
	tempV[2].position.y = tempV[2].position.x = 1;
	tempV[2].TexC = { 1,0 };
	tempV[3].position.x = 1;
	tempV[3].TexC = { 1,1 };

	UINT32 tempI[6] = { 0,1,2, 0, 2, 3 };

	{
		RenderItem* tempR = new RenderItem;
		tempR->mesh = new Mesh;
		tempR->mesh->Init(mDevice, &testTex, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, tempV, _countof(tempV), tempI, 6);
		mapRenderer.AddRenderItem(tempR);

		GameObject temp;
		auto& tempP = temp.transform.position;
		tempP.x = 5;
		tempP.y = tempP.z = 0;
		temp.transform.scale = 4;
		temp.item = tempR;
		mapRenderer.AddGameObject(temp);
	}

	//Tiles
	{
		Vertex tempV[4];

		tempV[0].position.x = tempV[0].position.y = tempV[0].position.z = 0;
		tempV[0].normal = { 0.572276, 0.816877, 0.0721907 };
		tempV[0].TexC = { 0,1 };
		tempV[1] = tempV[2] = tempV[3] = tempV[0];

		tempV[1].position.z = 1;
		tempV[1].TexC = { 0,0 };
		tempV[2].position.z = tempV[2].position.x = 1;
		tempV[2].TexC = { 1,0 };
		tempV[3].position.x = 1;
		tempV[3].TexC = { 1,1 };

		RenderItem* temp = new RenderItem;
		temp->mesh = new Mesh;
		temp->mesh->Init(mDevice, &testTex, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, tempV, _countof(tempV), tempI, 6);
		mapRenderer.AddRenderItem(temp);

		const float tttttt = 100;
		float div = 3;
		
		int count = 0;
		GameObject obj;
		obj.item = temp;
		auto& position = obj.transform.position;
		obj.transform.scale = div;

		for (float i = -tttttt; i < tttttt; i+= div)
			for (float j = -tttttt; j < tttttt; j+= div)
			{
				position.x = i;
				position.y = 0;
				position.z = j;
				mapRenderer.AddGameObject(obj);
				count++;
			}
		std::cout << '\n' << count << '\n';
	}

	{
		RenderItem* tempR = new RenderItem;
		tempR->mesh = new Mesh;
		tempR->mesh->Init(mDevice, &texWirefence, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, tempV, _countof(tempV), tempI, 6);
		mapRenderer.AddRenderItem(tempR);

		GameObject temp;
		temp.item = tempR;
		auto& tempP = temp.transform.position;
		tempP.x = 7;
		tempP.y = tempP.z = 30;
		temp.transform.scale = 7;
		mapRenderer.AddGameObject(temp);
	}

	{

		Vertex tempV[4];

		tempV[0].position.x = tempV[0].position.y = tempV[0].position.z = 0;
		tempV[0].normal = { 0.572276, 0.816877, 0.0721907 };
		tempV[0].TexC = { 0,1 };
		tempV[1] = tempV[2] = tempV[3] = tempV[0];

		tempV[1].position.y = 1;
		tempV[1].TexC = { 0,0 };
		tempV[2].position.y = tempV[2].position.x = 1;
		tempV[2].TexC = { 1,0 };
		tempV[3].position.x = 1;
		tempV[3].TexC = { 1,1 };

		UINT32 tempI[6] = { 0,1,2, 0, 2, 3 };

		const UINT vbByteSize = (UINT)4 * sizeof(Vertex);

		const UINT ibByteSize = (UINT)6 * sizeof(std::int32_t);


		auto geo = new Mesh;

		geo->Init(mDevice, nullptr, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, tempV, _countof(tempV), tempI, 6);

		{
			auto skullRitem = new RenderItem;
			skullRitem->mesh = geo;

			GameObject obj;
			obj.item = skullR;


			// Generate instance data.
			const int n = 5;
			int mInstanceCount = n * n * n;


			float width = 200.0f;
			float height = 200.0f;
			float depth = 200.0f;

			float x = -0.5f * width + 3;
			float y = -0.5f * height;
			float z = -0.5f * depth;
			float dx = width / (n - 1);
			float dy = height / (n - 1);
			float dz = depth / (n - 1);
			for (int k = 0; k < n; ++k)
			{
				for (int i = 0; i < n; ++i)
				{
					for (int j = 0; j < n; ++j)
					{
						int index = k * n * n + i * n + j;
						// Position instanced along a 3D grid.

						obj.transform.position.x = x + j * dx + 2;
						obj.transform.position.y = y + i * dy;
						obj.transform.position.z = z + k * dz;

						obj.transform.scale = 1;

						mapRenderer.AddGameObject(obj);
					}
				}
			}

			mapRenderer.AddRenderItem(skullRitem);
		}


	}

	{//TODO 
		boxData.speed = 1;
		boxData.type = RatsCraft::GameObjectType::TESTTYPE_BOX;

		gameObj.Init(&boxData);
		gameObj.forward = RatsCraft::Location(40, 40);

		m_game.m_gameObjs.push_back(gameObj);

		
		mapRenderer.m_objTypeToRenderItem[boxData.type] = skullR;
		//mapRenderer.AddGameObject(gameObj);
	}

	cout << "END\n";
}

void Dx::FlushCommandQueue()
{
	auto fence = mQueue.Signal();
	mQueue.WaitUntil(fence);
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
		else
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
	auto& cmdListAlloc = mCommandAllocator[currFrameResourceIndex];

	DxThrowIfFailed(cmdListAlloc->Reset());
	DxThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), nullptr));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	D3D12_RESOURCE_BARRIER barrier;
	Transition(barrier, CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	rtvHandle.ptr = mRtvHeap->GetCPUDescriptorHandleForHeapStart().ptr + mCurrBackBuffer * mRtvDescriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE dsView = DepthStencilView();
	mCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsView);

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DX::Colors::LightSteelBlue, 0, nullptr);

	//for(l: Layers) (ex. Game layer, UI layer......)
	{
		mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		mapRenderer.Draw(mCommandList, mQueue);
	}

	Transition(barrier, CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &barrier);

	DxThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mQueue.commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	DxThrowIfFailed(mSwapChain->Present(0, 0));

	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
	
	mCmdAllocsFence[currFrameResourceIndex] = mQueue.Signal();
	currFrameResourceIndex = (currFrameResourceIndex + 1) % FrameResource::FrameResources;
}

void Dx::OnResize()
{
	FlushCommandQueue();

	mMainCamera.SetProj(graphicSetting.width, graphicSetting.height);

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(graphicSetting.width);
	mScreenViewport.Height = static_cast<float>(graphicSetting.height);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	mScissorRect = { 0, 0, graphicSetting.width, graphicSetting.height };

	DxThrowIfFailed(mCommandList->Reset(mCommandAllocator[0].Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
		mRenderTargets[i].Reset();
	mDepthStencilBuffer.Reset();

	// Resize the swap chain.
	DxThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		graphicSetting.width, graphicSetting.height,
		graphicSetting.backBufferFormat,
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
	depthStencilDesc.Width = graphicSetting.width;
	depthStencilDesc.Height = graphicSetting.height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = graphicSetting.mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = graphicSetting.m4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = graphicSetting.m4xMsaaState ? (graphicSetting.m4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = graphicSetting.mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	D3D12_HEAP_PROPERTIES properties;
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
	dsvDesc.Format = graphicSetting.mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	D3D12_RESOURCE_BARRIER barrier;
	Transition(barrier, mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	mCommandList->ResourceBarrier(1, &barrier);

	DxThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mQueue.commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void Dx::Update(const GameTimer& gt)
{
	mQueue.WaitUntil(mCmdAllocsFence[currFrameResourceIndex]);
	m_game.Update(gt.DeltaTime());
	mapRenderer.Update(gt, mQueue);
}

void Dx::UpdateGraphicSetting(GraphicSetting& setting)
{
	CCreateSwapChain();
	OnResize();
}