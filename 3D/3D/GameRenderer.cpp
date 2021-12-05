#include "GameRenderer.h"

#include <d3dcompiler.h>

#include "tempd3dx.h"
#include "CD3DX12_ROOT_PARAMETER.h"

D3D12_GPU_DESCRIPTOR_HANDLE GameRenderer::lastTexHandle = { 0, };

HRESULT GameRenderer::Init(GraphicSetting& setting, ComPtr<ID3D12Device>& device)
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
		mFrameResources[i].ObjectDataBuffer.Init(device.Get(), 100000, false);
		mFrameResources[i].MaterialBuffer.Init(device.Get(), 2, true);

		mFrameResources[i].MaterialBuffer.CopyToBuffer(0, &tetst);
		mFrameResources[i].MaterialBuffer.CopyToBuffer(1, &shadow);

		mFrameResources[i].frameResource.AmbientLight = { 0.25f,0.25f,0.35f,1.0f };
		mFrameResources[i].frameResource.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		mFrameResources[i].frameResource.Lights[0].Strength = { 0.6f,0.6f,0.6f };
		mFrameResources[i].frameResource.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		mFrameResources[i].frameResource.Lights[1].Strength = { 0.3f,0.3f,0.3f };
		mFrameResources[i].frameResource.Lights[2].Direction = { 0.0f,-0.707f,-0.707f };
		mFrameResources[i].frameResource.Lights[2].Strength = { 0.15f,0.15f,0.15f };
	}

	DefineSkullAnimation();

	return S_OK;
}

void GameRenderer::DefineSkullAnimation()
{
	using namespace DX;

	XMVECTOR q0 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(30.0f));
	XMVECTOR q1 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 1.0f, 2.0f, 0.0f), XMConvertToRadians(45.0f));
	XMVECTOR q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(-30.0f));
	XMVECTOR q3 = XMQuaternionRotationAxis(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), XMConvertToRadians(70.0f));

	mSkullAnimation.Keyframes.resize(5);
	mSkullAnimation.Keyframes[0].TimePos = 0.0f;
	mSkullAnimation.Keyframes[0].Translation = XMFLOAT3(-7.0f, 0.0f, 0.0f);
	mSkullAnimation.Keyframes[0].Scale = XMFLOAT3(0.25f, 0.25f, 0.25f);
	XMStoreFloat4(&mSkullAnimation.Keyframes[0].RotationQuat, q0);

	mSkullAnimation.Keyframes[1].TimePos = 2.0f;
	mSkullAnimation.Keyframes[1].Translation = XMFLOAT3(0.0f, 2.0f, 10.0f);
	mSkullAnimation.Keyframes[1].Scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
	XMStoreFloat4(&mSkullAnimation.Keyframes[1].RotationQuat, q1);

	mSkullAnimation.Keyframes[2].TimePos = 4.0f;
	mSkullAnimation.Keyframes[2].Translation = XMFLOAT3(7.0f, 0.0f, 0.0f);
	mSkullAnimation.Keyframes[2].Scale = XMFLOAT3(0.25f, 0.25f, 0.25f);
	XMStoreFloat4(&mSkullAnimation.Keyframes[2].RotationQuat, q2);

	mSkullAnimation.Keyframes[3].TimePos = 6.0f;
	mSkullAnimation.Keyframes[3].Translation = XMFLOAT3(0.0f, 1.0f, -10.0f);
	mSkullAnimation.Keyframes[3].Scale = XMFLOAT3(0.5f, 0.5f, 0.5f);
	XMStoreFloat4(&mSkullAnimation.Keyframes[3].RotationQuat, q3);

	mSkullAnimation.Keyframes[4].TimePos = 8.0f;
	mSkullAnimation.Keyframes[4].Translation = XMFLOAT3(-7.0f, 0.0f, 0.0f);
	mSkullAnimation.Keyframes[4].Scale = XMFLOAT3(0.25f, 0.25f, 0.25f);
	XMStoreFloat4(&mSkullAnimation.Keyframes[4].RotationQuat, q0);
}


void GameRenderer::Update(const GameTimer& gt, GPUQueue& queue)
{
	mAnimTimePos += gt.DeltaTime();
	if (mAnimTimePos >= mSkullAnimation.GetEndTime())
		mAnimTimePos = 0.0f;

	currFrameResourceIndex = (currFrameResourceIndex + 1) % FrameResource::FrameResources;
	mCurrFrameResource = &mFrameResources[currFrameResourceIndex];

	queue.WaitUntil(mCurrFrameResource->fence);

	UpdateFrameResource(gt);

	{
		/*TODO Animation
				mAnimTimePos += gt.DeltaTime();
		if (mAnimTimePos >= mSkullAnimation.GetEndTime())
			mAnimTimePos = 0.0f;
		mSkullAnimation.Interpolate(mAnimTimePos, mapData.objs[0]._world);
		*/
	}

	UpdateGameObjectsMatrix(mapData.objs.list);
}

void GameRenderer::Draw(ComPtr<ID3D12GraphicsCommandList> cmdList, GPUQueue& queue)
{
	auto& frameCB = mCurrFrameResource->mFrameCB;
	auto& materialCB = mCurrFrameResource->MaterialBuffer;

	cmdList->SetPipelineState(mPSO.Get());
	cmdList->SetGraphicsRootSignature(mRootSignature.Get());

	auto& descriptorHeap = CTextureManager::GetInstance()->mSrvDescriptorHeap;
	ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeap.Get() };
	cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	cmdList->SetGraphicsRootShaderResourceView(1, materialCB.mBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(2, frameCB.mBuffer->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(3, descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	DrawRenderItems(cmdList, renderItems);

	mCurrFrameResource->fence = queue.Signal();
}

void GameRenderer::DrawRenderItems(ComPtr<ID3D12GraphicsCommandList>& cmdList, std::vector<RenderItem*>& renderItems)
{
	cmdList->SetGraphicsRootShaderResourceView(0, mCurrFrameResource->ObjectDataBuffer.mBuffer->GetGPUVirtualAddress());

	for (size_t i = 0; i < renderItems.size(); ++i)
	{
		auto& renderItem = renderItems[i];
		auto& mesh = renderItem->mesh;

		cmdList->IASetPrimitiveTopology(mesh->topology);
		cmdList->IASetIndexBuffer(&mesh->mIndexBufferView);
		cmdList->IASetVertexBuffers(0, 1, &mesh->mVertexBufferView);

		cmdList->DrawIndexedInstanced(mesh->indexes, renderItem->instances, 0, 0, renderItem->gpuIndex);
	}

	/*
	
	cmdList->SetDescriptorHeaps(1, CTextureManager::GetInstance()->mSrvDescriptorHeap.GetAddressOf());
	for (UINT i = 0; i < renderItems.size(); i++)
	{
		auto& renderItem = renderItems[i];

		D3D12_GPU_DESCRIPTOR_HANDLE texHandle = renderItem.mesh->mTex->_handle;
		if (lastTexHandle.ptr != texHandle.ptr)
		{
			cmdList->SetGraphicsRootDescriptorTable(0, texHandle);
			lastTexHandle = texHandle;
		}

		cmdList->SetGraphicsRootConstantBufferView(1, objCB.mBuffer->GetGPUVirtualAddress() + objCB.ElementSize() * renderItem._index);
		auto& mesh = renderItem.mesh;
		cmdList->IASetPrimitiveTopology(mesh->topology);
		cmdList->IASetIndexBuffer(&mesh->mIndexBufferView);
		cmdList->IASetVertexBuffers(0, 1, &mesh->mVertexBufferView);
		cmdList->DrawIndexedInstanced(mesh->indexes, 1, 0, 0, 0);
	}
	*/
}

void GameRenderer::AddRenderItem(RenderItem* item)
{
	renderItems.push_back(item);
}

void GameRenderer::AddGameObject(GameObject& item)
{
	mapData.objs.Add(item);
}

void GameRenderer::AddConstRenderObject(RenderItem& item)
{
	DxThrowIfFailed(-1);
	//TODO
	return;	
}

void GameRenderer::ChangeGraphicsSetting(GraphicSetting& setting)
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
	mContainsMirror = setting.mirror;
}

void GameRenderer::UpdateGameObjectsMatrix(std::vector<GameObject>& list)
{
	for (auto& item : renderItems)
		item->instances = 0;

	int visibleInstanceCount = 0;
	int len = list.size();
	for (int i = 0; i<len; i++)
	{
		auto& startItem = list[i];
		if (startItem.item == nullptr)//no render
			continue;

		RenderItem* renderItem = startItem.item;

		InstanceData data;

		renderItem->gpuIndex = visibleInstanceCount;

		int startIndex = visibleInstanceCount;
		for (; i < len; i++)
		{
			auto& item = list[i];
			if (item.item != startItem.item)
			{
				i--;
				break;
			}

			auto& transform = item.transform;
			auto& position = transform.position;

			data.World = DX::XMMatrixScaling(transform.scale, transform.scale, transform.scale) * DX::XMMatrixTranslation(position.x, position.y, position.z);
			data.World = DX::XMMatrixTranspose(data.World);
			data.TexTransform = DX::XMMatrixTranspose(DX::XMMatrixIdentity());//TODO
			data.MaterialIndex = 0;//TODO  instanceData[i].MaterialIndex;

			//XMStoreFloat4x4(&skullRitem->Instances[index].TexTransform, XMMatrixScaling(2.0f, 2.0f, 1.0f));

			//TODO : Check object is in Camera..// AABB

			mCurrFrameResource->ObjectDataBuffer.CopyToBuffer(visibleInstanceCount, &data);

			visibleInstanceCount++;
		}
		
		renderItem->instances = visibleInstanceCount - startIndex;
	}
	return;
	/*
	
	auto currInstanceBuffer = mCurrFrameResource->ObjectDataBuffer;
	for (auto& e : mAllRitems)
	{
		const auto& instanceData = e->Instances;

		int visibleInstanceCount = 0;

		for (UINT i = 0; i < (UINT)instanceData.size(); ++i)
		{
			DX::XMMATRIX world = XMLoadFloat4x4(&instanceData[i].World);
			DX::XMMATRIX texTransform = XMLoadFloat4x4(&instanceData[i].TexTransform);

			DX::XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(world), world);

			// View space to the object's local space.
			DX::XMMATRIX viewToLocal = XMMatrixMultiply(invView, invWorld);

			// Transform the camera frustum from view space to the object's local space.
			DX::BoundingFrustum localSpaceFrustum;
			mCamFrustum.Transform(localSpaceFrustum, viewToLocal);

			// Perform the box/frustum intersection test in local space.
			if ((localSpaceFrustum.Contains(e->Bounds) != DirectX::DISJOINT) || (mFrustumCullingEnabled == false))
			{
				InstanceData data;
				XMStoreFloat4x4(&data.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&data.TexTransform, XMMatrixTranspose(texTransform));
				data.MaterialIndex = instanceData[i].MaterialIndex;

				// Write the instance data to structured buffer for the visible objects.
				currInstanceBuffer->CopyData(visibleInstanceCount++, data);
			}
		}

		e->InstanceCount = visibleInstanceCount;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"Instancing and Culling Demo" <<
			L"    " << e->InstanceCount <<
			L" objects visible out of " << e->Instances.size();
		mMainWndCaption = outs.str();
	}
	*/
}

void GameRenderer::UpdateFrameResource(const GameTimer& gt)
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

void GameRenderer::BuildRootSignature()
{
	D3D12_DESCRIPTOR_RANGE texTable;
	texTable.BaseShaderRegister = 0;//Register t0
	texTable.NumDescriptors = 7;
	texTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	texTable.RegisterSpace = 0;
	texTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	enum { slots = 4 };
	CD3DX12_ROOT_PARAMETER slotRootParameter[slots];
	slotRootParameter[0].InitAsShaderResourceView(0, 1);
	slotRootParameter[1].InitAsShaderResourceView(1, 1);
	slotRootParameter[2].InitAsConstantBufferView(0);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

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

void GameRenderer::BuildPSO()
{
#if CDEBUG
	UINT compileFlags = 0;
#else
	UINT compileFlags = 0;
#endif

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	DxThrowIfFailed(D3DCompileFromFile(L"Shaders\\Default.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS", "vs_5_1", compileFlags, 0, &vertexShader, nullptr));
	DxThrowIfFailed(D3DCompileFromFile(L"Shaders\\Default.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS", "ps_5_1", compileFlags, 0, &pixelShader, nullptr));

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

	/*
		ComPtr<ID3DBlob> computeShader;
	//DxThrowIfFailed(D3DCompileFromFile(L"computeShader.txt", nullptr, nullptr, "CS", "cs_5_0", compileFlags, 0, &vertexShader, nullptr));

	D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
	desc.pRootSignature = mRootSignature.Get();
	desc.CS.BytecodeLength = computeShader->GetBufferSize();
	desc.CS.pShaderBytecode = computeShader->GetBufferPointer();
	desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	//DxThrowIfFailed(mDevice->CreateComputePipelineState(&desc, IID_PPV_ARGS(&mPsoCompute)));

	*/
	
}