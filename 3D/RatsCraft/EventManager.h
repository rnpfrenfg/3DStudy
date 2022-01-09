#pragma once

#include <vector>

#include "EventType.h"
#include "EventListener.h"

namespace RatsCraft
{
	class RatsCraftAPI EventManager
	{
	public:
		EventManager() = default;
		~EventManager() = default;

		void AddListener(EventListener* listener, EventType type);
		void RemoveListener(EventListener* listener, EventType type);
		void RemoveAll(EventListener* pListener);

		void TriggerEvent(EventType type, void* eventData);

	private:
		EventManager(const EventManager&) = default;
		EventManager& operator=(const EventManager&) = default;

		std::vector<EventListener*> m_listenersVec[static_cast<int>(EventType::NUM_EVENTS)];
	};
}