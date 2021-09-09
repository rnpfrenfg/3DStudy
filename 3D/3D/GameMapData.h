#pragma once

#include "include.h"

#include "RenderItem.h"

struct GameMapData
{
	std::vector<RenderItem> mObjs;
	std::vector<RenderItem> mConstants;
	std::vector<RenderItem> mTranses;
	std::vector<RenderItem> mMirrors;
	std::vector<RenderItem> mReflectedObjs;
	std::vector<RenderItem> mShwdowObjs;
};