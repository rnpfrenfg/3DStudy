#pragma once

#include "RenderItem.h"

class Vector3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;
};

class Transform
{
public:
	int scale = 1;
	Vector3 position;
};

class GameObject
{
public:
	Transform transform;
	RenderItem* item = nullptr;

	//for sort
	bool operator<(const GameObject& obj) const
	{
		return item < obj.item;
	}

private: //TODO make
	UINT dirty = FrameResource::FrameResources;
	DX::XMMATRIX world;//dirty


};

class GameObjectList
{
public:
	void Add(GameObject obj)
	{
		list.push_back(obj);
		sort(list.begin(), list.end());
	}

	std::vector<GameObject> list;
};