#include "includeAtCPP.h"
#include "EventManager.h"

namespace TetrisSpace
{
	void EventManager::PushEvent(EventType et, void* data)
	{
		for (EventFunction func : funcList[et])
			func(data);
	}

	void EventManager::AddEventListener(EventType et, EventFunction func)
	{
		auto iter = funcList.find(et);
		if (iter == funcList.end())
		{
			std::vector<EventFunction> vec;
			vec.push_back(func);
			funcList.insert({ et, vec });
		}
		else
		{
			funcList[et].push_back(func);
		}
	}
}