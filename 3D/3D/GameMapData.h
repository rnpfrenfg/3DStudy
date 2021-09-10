#pragma once

#include "include.h"

#include "RenderItem.h"

struct GameMapData
{
	std::vector<RenderItem> objs;
	std::vector<RenderItem> constObjs;
	std::vector<RenderItem> transparents;
	std::vector<RenderItem> mirrors;

	std::vector<RenderItem> _reflected;
	std::vector<RenderItem> _shadows;
};