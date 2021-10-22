#pragma once

#include "include.h"

struct GraphicSetting
{
	bool m4xMsaaState = false;
	UINT m4xMsaaQuality = 0;

	bool shadow = false;
	bool mirror = false;

	int width = 800;
	int height = 600;

	//for directX

	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
};

/* 
Display mode
Aspect ratio
Resolution
Brightnes


Field Of View // Camera
FovAbillityScaling


SprintViewShake

{
V-sync
NVidia Reflex
AdaptiveResolutionFPSTarget
AdaptiveSupersampling
Anti-aliasing
TextureStreamingBudget
TextureFiltering
AmbientOcclusionQuality
Sun Shadow Coverage
Sun Shadow Detail
Spot Shadow Detail
Volumetric Lighting
Dynamic Spot Shadows
Model Detail
Effects Detail
Impact Marks
Ragdolls
}
*/