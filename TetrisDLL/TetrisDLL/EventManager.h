#pragma once

#include "include.h"

#include <unordered_map>
#include <vector>

#include "EventType.h"
#include "EventListener.h"

namespace TetrisSpace
{
	class TETRISAPI EventManager
	{
	public:
		EventManager() = default;
		~EventManager() = default;

		void TriggerEvent(EventType type, void* eventData);

		void AddListener(EventListener* listener, EventType type);
		void RemoveListener(EventListener* listener, EventType type);
		void RemoveAll(EventListener* pListener);

	private:
		EventManager(const EventManager&) = default;
		EventManager& operator=(const EventManager&) = default;

		std::vector<EventListener*> mListenersVec[static_cast<int>(EventType::NUM_EVENTS)];
	};
}