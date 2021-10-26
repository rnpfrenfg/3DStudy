#pragma once

#include "include.h"

#include <unordered_map>
#include <vector>

#include "EventType.h"

namespace TetrisSpace
{
	typedef void (*EventFunction)(void* data);

	class TETRISAPI EventManager
	{
	public:
		EventManager() = default;
		~EventManager() = default;

		void PushEvent(EventType et, void* data);

		void AddEventListener(EventType et, EventFunction func);
	
	private:
		std::unordered_map<EventType, std::vector<EventFunction>> funcList;
	};
}

