#pragma once

#include "UploadBuffer.h"

#include "Light.h"

struct InstanceData
{
	DX::XMMATRIX World = DX::XMMatrixIdentity();
	DX::XMMATRIX TexTransform = DX::XMMatrixIdentity();
	UINT MaterialIndex;
	UINT InstancePad0;
	UINT InstancePad1;
	UINT InstancePad2;
};

struct MaterialConstants
{
	DX::XMFLOAT4 DiffuseAlbedo = { 1.0f,1.0f,1.0f,1.0f };
	DX::XMFLOAT3 FresnelR0 = { 0.01f,0.01f,0.01f };
	float Roughness = 0.25f;

	DX::XMMATRIX MatTransform = DX::XMMatrixScaling(1, 1, 1);
};

struct FrameConstants
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

class FrameResource
{
public:
	enum {FrameResources = 2};

	UploadBuffer<InstanceData> ObjectDataBuffer;
	UploadBuffer<MaterialConstants> MaterialBuffer;
	UploadBuffer<FrameConstants> mFrameCB;
	FrameConstants frameResource;

	UINT64 fence = 0;
};

