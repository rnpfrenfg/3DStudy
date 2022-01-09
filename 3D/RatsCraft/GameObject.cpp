#include "includeCpp.h"
#include "GameObject.h"

namespace RatsCraft
{
	GameObject::GameObject()
	{

	}

	GameObject::~GameObject()
	{

	}

	void GameObject::Init(GameObjectData* data)
	{
		m_objData = data;
	}

	void GameObject::Update(const float dt)
	{
		Location newLoc = forward;
		newLoc -= this->loc;
		
		newLoc.Normalize();
		
		newLoc.x *= dt;
		newLoc.y *= dt;
		newLoc.z *= dt;

		this->loc += newLoc;
	}
}