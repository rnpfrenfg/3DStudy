#include "GameMap.h"

#include <d3dcompiler.h>

#include "tempd3dx.h"
#include "CD3DX12_ROOT_PARAMETER.h"

HRESULT GameMap::Init(GraphicSetting& setting, ComPtr<ID3D12Device>& device)
{
	setting = graphicsSet;
	mDevice = device;

	ChangeGraphicsSetting(setting);

	BuildRootSignature();
	BuildPSO();

	MaterialConstants tetst;
	tetst.DiffuseAlbedo = DX::XMFLOAT4(1, 1, 1, 1);
	tetst.FresnelR0 = DX::XMFLOAT3(0.05f, 0.05f, 0.05f);
	tetst.Roughness = 0.3f;
	tetst.MatTransform = DX::XMMatrixIdentity();
	tetst.MatTransform = DX::XMMatrixTranspose(tetst.MatTransform);

	MaterialConstants shadow;
	shadow.DiffuseAlbedo = DX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	shadow.FresnelR0 = DX::XMFLOAT3(0.001f, 0.001f, 0.001f);
	shadow.Roughness = 0;
	shadow.MatTransform = DX::XMMatrixIdentity();
	shadow.MatTransform = DX::XMMatrixTranspose(tetst.MatTransform);

	for (int i = 0; i < FrameResource::FrameResources; i++)
	{
		mFrameResources[i].mFrameCB.Init(device.Get(), 2, true);
		mFrameResources[i].mObjectCB.Init(device.Get(), 100000, true);
		mFrameResources[i].mMaterialTestCB.Init(device.Get(), 2, true);

		mFrameResources[i].mMaterialTestCB.CopyToBuffer(0, &tetst);
		mFrameResources[i].mMaterialTestCB.CopyToBuffer(1, &shadow);

		mFrameResources[i].frameResource.AmbientLight = { 0.25f,0.25f,0.35f,1.0f };
		mFrameResources[i].frameResource.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		mFrameResources[i].frameResource.Lights[0].Strength = { 0.6f,0.6f,0.6f };
		mFrameResources[i].frameResource.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		mFrameResources[i].frameResource.Lights[1].Strength = { 0.3f,0.3f,0.3f };
		mFrameResources[i].frameResource.Lights[2].Direction = { 0.0f,-0.707f,-0.707f };
		mFrameResources[i].frameResource.Lights[2].Strength = { 0.15f,0.15f,0.15f };

	}

	return S_OK;
}

void GameMap::AddConstRenderObject(RenderItem& item)
{
	_AddItem(item, mPSO, mapData.constObjs);
	for (int i = 0; i < FrameResource::FrameResources; i++)
	{
		UpdateMatrix(mapData.constObjs[mapData.constObjs.size() - 1], mFrameResources[i].mObjectCB);
	}
}

void GameMap::Update(const GameTimer& gt, GPUQueue& queue)
{
	currFrameResourceIndex = (currFrameResourceIndex + 1) % FrameResource::FrameResources;
	mCurrFrameResource = &mFrameResources[currFrameResourceIndex];

	queue.WaitUntil(mCurrFrameResource->fence);

	UpdateFrameResource(gt);

	auto& objCB = mCurrFrameResource->mObjectCB;

	for (RenderItem& item : mapData.objs)
		UpdateMatrix(item, objCB);
	for (RenderItem& item : mapData.transparents)
		UpdateMatrix(item, objCB);
	for (RenderItem& item : mapData.mirrors)
		UpdateMatrix(item, objCB);
	for (RenderItem& item : mapData._shadows)
		UpdateShadowMatrix(item, objCB);
	for (RenderItem& item : mapData._reflected)
		UpdateReflectedMatrix(item, objCB);
}

void GameMap::Draw(ComPtr<ID3D12GraphicsCommandList> cmdList, GPUQueue& queue)
{
	auto& frameCB = mCurrFrameResource->mFrameCB;
	auto& materialCB = mCurrFrameResource->mMaterialTestCB;

	cmdList->SetGraphicsRootSignature(mRootSignature.Get());

	cmdList->SetGraphicsRootConstantBufferView(2, frameCB.mBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(3, materialCB.mBuffer->GetGPUVirtualAddress());

	cmdList->SetPipelineState(mPSO.Get());

	DrawRenderItems(cmdList, mapData.objs);
	DrawRenderItems(cmdList, mapData.constObjs);

	if (mContainsMirror)
	{
		cmdList->OMSetStencilRef(1);
		cmdList->SetPipelineState(mPsoMarkStencilMirrors.Get());
		DrawRenderItems(cmdList, mapData.mirrors);

		cmdList->SetGraphicsRootConstantBufferView(2, frameCB.mBuffer->GetGPUVirtualAddress() + frameCB.ElementSize());
		cmdList->SetPipelineState(mPsoDrawReflections.Get());
		DrawRenderItems(cmdList, mapData._reflected);

		cmdList->SetGraphicsRootConstantBufferView(2, frameCB.mBuffer->GetGPUVirtualAddress());
		cmdList->OMSetStencilRef(0);
	}

	if (mShadow)
	{
		cmdList->SetGraphicsRootConstantBufferView(3, materialCB.mBuffer->GetGPUVirtualAddress() + materialCB.ElementSize());
		cmdList->SetPipelineState(mPsoBlend.Get());
		DrawRenderItems(cmdList, mapData._shadows);
	}

	mCurrFrameResource->fence = queue.Signal();
}

void GameMap::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList>& cmdList, std::vector<RenderItem>& meshObjects)
{
	auto& objCB = mCurrFrameResource->mObjectCB;
	for (UINT i = 0; i < meshObjects.size(); i++)
	{
		auto& mesh = meshObjects[i];

		static D3D12_GPU_DESCRIPTOR_HANDLE lasthandle = { 0, };//singleton
		D3D12_GPU_DESCRIPTOR_HANDLE texHandle = mesh.mesh->mTex->_handle;
		if (lasthandle.ptr != texHandle.ptr)
		{
			cmdList->SetGraphicsRootDescriptorTable(0,texHandle);
			lasthandle = texHandle;
		}
		cmdList->SetGraphicsRootConstantBufferView(1, objCB.mBuffer->GetGPUVirtualAddress() + objCB.ElementSize() * (mesh._index));

		cmdList->ExecuteBundle(mesh._bundle.mCommandList.Get());
	}
}

void GameMap::_AddItem(RenderItem& item, ComPtr<ID3D12PipelineState> pso, std::vector<RenderItem>& list)
{
	item._InitBundle(mDevice, pso);
	item._index = newObjIndex;
	item.dirty = FrameResource::FrameResources;
	list.push_back(item);
	newObjIndex++;
}

void GameMap::AddRenderObject(RenderItem& item)
{
	_AddItem(item, mPSO, mapData.objs);

	if (mContainsMirror)
	{
		_AddItem(item, mPsoDrawReflections, mapData._reflected);
	}
	if (mShadow)
	{
		_AddItem(item, mPsoShadow, mapData._shadows);
	}
}

void GameMap::AddTransparent(RenderItem& item)
{
	_AddItem(item, mPsoBlend, mapData.transparents);
}

void GameMap::ChangeGraphicsSetting(GraphicSetting& setting)
{
	graphicsSet = setting;

	for (int i = 0; i < FrameResource::FrameResources; i++)
	{
		auto& frameResource = mFrameResources[i].frameResource;
		frameResource.RenderTargetSize = DX::XMFLOAT2(graphicsSet.width, graphicsSet.height);
		frameResource.InvRenderTargetSize = DX::XMFLOAT2(1.0f / graphicsSet.width, 1.0f / graphicsSet.height);
	}

	if (setting.shadow && !mShadow)
	{
		//TODO
	}
	mShadow = setting.shadow;

	mContainsMirror = true;
}

void GameMap::UpdateMatrix(RenderItem& item, UploadBuffer<ObjectConstants>& objectCB)
{
	if (item.dirty)
	{
		ObjectConstants objConst;
		DX::XMMATRIX m;

		m = DX::XMMatrixScaling(item.scale, item.scale, item.scale) * DX::XMMatrixTranslation(item.x, item.y, item.z);
		m = DX::XMMatrixTranspose(m);

		objConst.world = m;
		objConst.TexTransform = DX::XMMatrixTranspose(objConst.TexTransform);
		objectCB.CopyToBuffer(item._index, &objConst);
		item.dirty--;
	}
}

void GameMap::UpdateShadowMatrix(RenderItem& item, UploadBuffer<ObjectConstants>& objectCB)
{
	if (item.dirty)
	{
		DX::XMVECTOR shadowPlane = DX::XMVectorSet(0, 1, 0, 0);

		DX::XMFLOAT3 light = mCurrFrameResource->frameResource.Lights[0].Direction;
		light.x = -light.x;
		light.y = -light.y;
		light.z = -light.z;
		DX::XMVECTOR toMainLight = DX::XMLoadFloat3(&light);

		DX::XMMATRIX S = DX::XMMatrixShadow(shadowPlane, toMainLight);
		DX::XMMATRIX shadowOffsetY = DX::XMMatrixTranslation(0, 0.001f, 0);

		ObjectConstants objConst;
		DX::XMMATRIX m;

		m = DX::XMMatrixScaling(item.scale, item.scale, item.scale) * DX::XMMatrixTranslation(item.x, item.y, item.z);
		m *= S * shadowOffsetY;
		m = DX::XMMatrixTranspose(m);

		objConst.world = m;
		objConst.TexTransform = DX::XMMatrixTranspose(objConst.TexTransform);
		objectCB.CopyToBuffer(item._index, &objConst);
		item.dirty--;
	}
}
void GameMap::UpdateReflectedMatrix(RenderItem& item, UploadBuffer<ObjectConstants>& objectCB)
{
	if (item.dirty)
	{
		DX::XMVECTOR mirrorPlane = DX::XMVectorSet(0, 0, 1, 0);
		DX::XMMATRIX R = DX::XMMatrixReflect(mirrorPlane);

		ObjectConstants objConst;
		DX::XMMATRIX m;

		m = DX::XMMatrixScaling(item.scale, item.scale, item.scale) * DX::XMMatrixTranslation(item.x, item.y, item.z);
		m *= R;
		m = DX::XMMatrixTranspose(m);

		objConst.world = m;
		objConst.TexTransform = DX::XMMatrixTranspose(objConst.TexTransform);
		objectCB.CopyToBuffer(item._index, &objConst);
		item.dirty--;
	}
}

void GameMap::UpdateFrameResource(const GameTimer& gt)
{
	using DX::XMVECTOR;
	using DX::XMMATRIX;

	mMainCamera.SetView();

	XMMATRIX& view = mMainCamera.mView;
	XMMATRIX& proj = mMainCamera.mProj;

	XMMATRIX viewProj = mMainCamera.mView * mMainCamera.mProj;
	XMMATRIX invView = DX::XMMatrixInverse(nullptr, view);
	XMMATRIX invProj = DX::XMMatrixInverse(nullptr, proj);
	XMMATRIX invViewProj = DX::XMMatrixInverse(nullptr, viewProj);

	auto& frameResource = mCurrFrameResource->frameResource;

	frameResource.view = DX::XMMatrixTranspose(view);
	frameResource.InvView = DX::XMMatrixTranspose(invView);
	frameResource.Proj = DX::XMMatrixTranspose(proj);
	frameResource.InvProj = DX::XMMatrixTranspose(invProj);
	frameResource.ViewProj = DX::XMMatrixTranspose(viewProj);
	frameResource.InvViewProj = DX::XMMatrixTranspose(invViewProj);

	DX::XMStoreFloat3(&frameResource.EyePosW, mMainCamera.target);
	frameResource.NearZ = mMainCamera.mNearZ;
	frameResource.FarZ = mMainCamera.mFarZ;
	frameResource.TotalTime = gt.TotalTime();
	frameResource.DeltaTime = gt.DeltaTime();

	mCurrFrameResource->mFrameCB.CopyToBuffer(0, &frameResource);

	auto& reflectedFrameResoruce = mCurrFrameResource->reflectedFrameResoruce;
	reflectedFrameResoruce = frameResource;
	DX::XMVECTOR mirrorPlane = DX::XMVectorSet(0, 0, 1, 0);
	DX::XMMATRIX R = DX::XMMatrixReflect(mirrorPlane);

	for (int i = 0; i < 3; i++)
	{
		DX::XMVECTOR lightDir = XMLoadFloat3(&frameResource.Lights[i].Direction);
		DX::XMVECTOR reflectedLightDir = DX::XMVector3TransformNormal(lightDir, R);
		DX::XMStoreFloat3(&reflectedFrameResoruce.Lights[i].Direction, reflectedLightDir);
	}

	mCurrFrameResource->mFrameCB.CopyToBuffer(1, &reflectedFrameResoruce);
}

std::array<const D3D12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> arr = { 0, };


	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}


void GameMap::BuildRootSignature()
{
	enum { slots = 4 };
	CD3DX12_ROOT_PARAMETER slotRootParameter[slots];

	D3D12_DESCRIPTOR_RANGE texTable;
	texTable.BaseShaderRegister = 0;//Register t0
	texTable.NumDescriptors = 1;
	texTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	texTable.RegisterSpace = 0;
	texTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	D3D12_STATIC_SAMPLER_DESC desc;

	auto samplers = GetStaticSamplers();
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.NumParameters = slots;
	rootSigDesc.pParameters = slotRootParameter;
	rootSigDesc.NumStaticSamplers = samplers.size();
	rootSigDesc.pStaticSamplers = samplers.data();
	rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> serializedRootsig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	DxThrowIfFailed(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, serializedRootsig.GetAddressOf(), errorBlob.GetAddressOf()));
	DxThrowIfFailed(mDevice->CreateRootSignature(0, serializedRootsig->GetBufferPointer(), serializedRootsig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature)));
}

void GameMap::BuildPSO()
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
	psoDesc.RasterizerState.MultisampleEnable = FALSE;
	psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	psoDesc.RasterizerState.ForcedSampleCount = 0;
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
	psoDesc.DepthStencilState.StencilEnable = FALSE;
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
	psoDesc.RTVFormats[0] = graphicsSet.backBufferFormat;
	psoDesc.SampleDesc.Count = graphicsSet.m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = graphicsSet.m4xMsaaState ? (graphicsSet.m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = graphicsSet.mDepthStencilFormat;

	DxThrowIfFailed(mDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = psoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC blendDesc;
	blendDesc.BlendEnable = true;
	blendDesc.LogicOpEnable = false;
	blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.BlendState.RenderTarget[0] = blendDesc;
	DxThrowIfFailed(mDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPsoBlend)));

	D3D12_BLEND_DESC mirrorBlendState = psoDesc.BlendState;
	mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	D3D12_DEPTH_STENCIL_DESC mirrorDSS;
	mirrorDSS.DepthEnable = true;
	mirrorDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	mirrorDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	mirrorDSS.StencilEnable = true;
	mirrorDSS.StencilReadMask = 0xff;
	mirrorDSS.StencilWriteMask = 0xff;
	mirrorDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	mirrorDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	mirrorDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	mirrorDSS.BackFace = mirrorDSS.FrontFace;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC markMirrorsPsoDesc = psoDesc;
	markMirrorsPsoDesc.BlendState = mirrorBlendState;
	markMirrorsPsoDesc.DepthStencilState = mirrorDSS;
	markMirrorsPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	DxThrowIfFailed(mDevice->CreateGraphicsPipelineState(&markMirrorsPsoDesc, IID_PPV_ARGS(&mPsoMarkStencilMirrors)));

	D3D12_DEPTH_STENCIL_DESC reflectionDSS;
	reflectionDSS.DepthEnable = true;
	reflectionDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	reflectionDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	reflectionDSS.StencilEnable = true;
	reflectionDSS.StencilReadMask = 0xff;
	reflectionDSS.StencilWriteMask = 0xff;

	reflectionDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	reflectionDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	reflectionDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	reflectionDSS.BackFace = reflectionDSS.FrontFace;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC reflectionPsoDesc = psoDesc;
	reflectionPsoDesc.DepthStencilState = reflectionDSS;
	reflectionPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	reflectionPsoDesc.RasterizerState.FrontCounterClockwise = true;
	DxThrowIfFailed(mDevice->CreateGraphicsPipelineState(&reflectionPsoDesc, IID_PPV_ARGS(&mPsoDrawReflections)));

	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	shadowDSS.BackFace = shadowDSS.FrontFace;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = transparentPsoDesc;
	shadowPsoDesc.DepthStencilState = shadowDSS;
	DxThrowIfFailed(mDevice->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&mPsoShadow)));
}