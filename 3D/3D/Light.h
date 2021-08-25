#pragma once

#include "include.h"

struct Light
{
	DX::XMFLOAT3 Strength = { 0.5f,0.5f,0.5f };
	float FalloffStart = 1.0f;
	DX::XMFLOAT3 Direction = { 0.0f,-1.0f,0.0f };
	float FalloffEnd = 10.0f;
	DX::XMFLOAT3 Position = { 0,0,0 };
	float SpotPower = 64.0f;
};

#define MaxLights 16